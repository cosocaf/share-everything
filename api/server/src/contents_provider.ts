import { Express, Request, Response } from "express";
import { Collection } from "mongodb";
import { fileTypeFromBuffer } from "file-type";
import { AWSClient } from "./aws.js";
import { Room } from "./db.js";
import { Provider } from "./provider.js";

/**
 * /contents/以下を処理するプロバイダ。
 */
export class ContentsProvider extends Provider {
  constructor(private rooms: Collection<Room>, private awsClient: AWSClient) {
    super();
  }

  register(app: Express) {
    app.get("/contents/:id", (req, res) => void this.get(req, res));
  }

  private async get(
    req: Request<{
      id: string;
    }>,
    res: Response,
  ) {
    const id = req.params.id;

    console.log(`Requested get contents: ${id}`);

    try {
      const room = await this.rooms.findOne(
        { id },
        { projection: { mime: 1 } },
      );
      if (!room) {
        throw new Error("Invalid ID.");
      }

      const stream = await this.awsClient.download(`contents/${id}`);
      const content = [];
      for await (const value of stream) {
        content.push(value);
      }

      const buffer = Buffer.concat(content);
      const filetype = await fileTypeFromBuffer(buffer);
      if (filetype) {
        res.setHeader("Content-Type", filetype.mime);
      } else {
        res.setHeader("Content-Type", "application/octet-stream");
      }

      res.setHeader("Content-Length", buffer.byteLength);
      res.end(buffer);
    } catch (err) {
      this.handleError(err, res);
    }
  }
}
