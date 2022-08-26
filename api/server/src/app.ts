import express from "express";
import helmet from "helmet";
import cors from "cors";
import { MongoClient } from "mongodb";
import crypto from "crypto";
import createError from "http-errors";

// import { v4 } from "uuid";

const app = express();
app.use(helmet());
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

const protocol = process.env.DB_PROTOCOL || "";
const userName = process.env.DB_USER_NAME || "";
const password = process.env.DB_USER_PASSWORD || "";
const hostName = process.env.DB_HOST_NAME || "";
const dbName = process.env.DB_NAME || "";

const url = `${protocol}://${userName}:${password}@${hostName}/${dbName}?retryWrites=true&w=majority&authSource=admin`;
console.log(`Connecting: ${url}`);
const client = new MongoClient(url);

try {
  await client.connect();
  console.log("Succesfully connected to mongo.");
} catch (e) {
  console.error(e);
  process.exit(1);
}

const db = client.db();

type Room = {
  id: string;
  content: string;
};

const rooms = db.collection<Room>("rooms");

// TODO: express v5以降は非同期関数渡せるようになるらしいよ

const generateRoomId = () => {
  for (;;) {
    // これのためだけにcryptoつかうのはオーバースペックな気がする
    const id = crypto.randomBytes(6).toString("base64").substring(0, 6);
    if (id.includes("/")) continue;
    return id;
  }
};

app.post("/rooms", (_, res) => {
  // TODO: 認証機能とかあったほうがいいよね
  // const id = v4(); // UUIDとか長すぎてタイプが無理だから却下

  const id = generateRoomId();

  rooms
    .insertOne({ id, content: "" })
    .then(() => {
      console.log(`Room created: ${id}`);
      res.send({ status: "success", result: { id } });
    })
    .catch((err) => {
      console.error(err);
      if (err instanceof Error) {
        res.send({ status: "error", reason: err.message });
      } else {
        res.send({ status: "error", reason: "Unknown Error." });
      }
    });
});

app.get("/rooms/:id", (req, res) => {
  const id = req.params.id;

  console.log(`Requested get: ${id}`);

  rooms
    .findOne({ id }, { projection: { content: 1 } })
    .then((room) => {
      if (room) {
        console.log(`Found content(${id}): ${room.content}`);
        res.send({ status: "success", result: { content: room.content } });
      } else {
        throw new Error("Invalid ID.");
      }
    })
    .catch((err) => {
      console.error(err);
      if (err instanceof Error) {
        res.send({ status: "error", reason: err.message });
      } else {
        res.send({ status: "error", reason: "Unknown Error." });
      }
    });
});
app.put(
  "/rooms/:id",
  (req: express.Request<{ id: string }, unknown, { content: string }>, res) => {
    const id = req.params.id;
    const content = req.body.content;

    console.log(`Request put: ${id} = ${content}`);

    rooms
      .updateOne({ id }, { $set: { content } })
      .then(() => {
        res.send({ status: "success", result: {} });
      })
      .catch((err) => {
        console.error(err);
        if (err instanceof Error) {
          res.send({ status: "error", reason: err.message });
        } else {
          res.send({ status: "error", reason: "Unknown Error." });
        }
      });
  },
);
app.delete("/rooms/:id", (req, res) => {
  const id = req.params.id;

  console.log(`Requested delete: ${id}`);

  rooms
    .deleteOne({ id })
    .then(() => {
      res.send({ status: "success", result: {} });
    })
    .catch((err) => {
      console.error(err);
      if (err instanceof Error) {
        res.send({ status: "error", reason: err.message });
      } else {
        res.send({ status: "error", reason: "Unknown Error." });
      }
    });
});

app.use((_req, _res, next) => {
  return next(createError(404));
});

app.listen(8080, "0.0.0.0", () => {
  console.log("Listening on port 8080.");
});
