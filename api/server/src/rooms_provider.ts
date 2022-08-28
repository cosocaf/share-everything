import { Collection } from "mongodb";
import { Express, Request, Response } from "express";
import { Room } from "./db.js";
import { randomBytes } from "crypto";
import { AWSClient } from "./aws.js";
import { Provider } from "./provider.js";

const origin = process.env.ORIGIN || "";

/**
 * /rooms/以下を処理するプロバイダ。
 */
export class RoomsProvider extends Provider {
  constructor(private rooms: Collection<Room>, private awsClient: AWSClient) {
    super();
  }

  register(app: Express) {
    app.post("/rooms", (req, res) => void this.post(req, res));
    app.get("/rooms/:id", (req, res) => void this.get(req, res));
    app.put("/rooms/:id", (req, res) => void this.put(req, res));
    app.delete("/rooms/:id", (req, res) => void this.delete(req, res));
  }

  private async post(_: Request, res: Response) {
    console.log("Requested post");

    const generateRoomId = () => {
      for (;;) {
        // これのためだけにcryptoつかうのはオーバースペックな気がする
        const id = randomBytes(6).toString("base64").substring(0, 6);
        // idに/が入ってるとURLが壊れる
        if (id.includes("/")) continue;
        return id;
      }
    };
    const id = generateRoomId();
    console.log(`Generated new id: ${id}`);

    try {
      await this.rooms.insertOne({ id, content: "" });
      console.log(`Room created: ${id}`);
      res.send({ status: "success", result: { id } });
    } catch (err) {
      this.handleError(err, res);
    }
  }

  private async get(
    req: Request<{
      id: string;
    }>,
    res: Response,
  ) {
    const id = req.params.id;

    console.log(`Requested get: ${id}`);

    try {
      const room = await this.rooms.findOne(
        { id },
        { projection: { content: 1 } },
      );
      if (room) {
        console.log(`Found content(${id}): ${room.content}`);
        res.send({ status: "success", result: { content: room.content } });
      } else {
        throw new Error("Invalid ID.");
      }
    } catch (err) {
      this.handleError(err, res);
    }
  }

  private async put(
    req: Request<
      { id: string },
      unknown,
      { content: string; type: string; format: string }
    >,
    res: Response,
  ) {
    const id = req.params.id;
    const content = req.body.content;
    const type = req.body.type || "text";
    const format = req.body.format || "raw";

    console.log(`Requested put: id=${id}, type=${type}`);

    try {
      let decoded: Buffer;
      if (format === "raw") {
        decoded = Buffer.from(content, "utf8");
      } else if (format === "base64") {
        decoded = Buffer.from(content, "base64");
      } else {
        throw new Error(`Invalid format: ${format}`);
      }

      if (type === "text") {
        console.log(`Text content: ${content}`);
        await this.rooms.updateOne(
          { id },
          { $set: { content: decoded.toString() } },
        );
        res.send({ status: "success", result: {} });
      } else if (type === "url") {
        await this.awsClient.upload(`contents/${id}`, decoded);
        await this.rooms.updateOne(
          { id },
          { $set: { content: `${origin}/contents/${id}` } },
        );
        res.send({ status: "success", result: {} });
      } else {
        res.status(403);
      }
    } catch (err) {
      this.handleError(err, res);
    }
  }

  private async delete(
    req: Request<{
      id: string;
    }>,
    res: Response,
  ) {
    const id = req.params.id;

    console.log(`Requested delete: ${id}`);

    try {
      await this.rooms.deleteOne({ id });
      res.send({ status: "success", result: {} });
    } catch (err) {
      this.handleError(err, res);
    }
  }
}
