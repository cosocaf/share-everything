import express from "express";
import helmet from "helmet";
import cors from "cors";
import { MongoClient } from "mongodb";

import { v4 } from "uuid";

const app = express();
app.use(helmet());
app.use(cors());
app.use(express.json());

const client = new MongoClient(
  "mongodb://admin:admin@mongo/share-everything?authSource=admin",
);

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

app.post("/rooms", (_, res) => {
  // TODO: 認証機能とかあったほうがいいよね
  const id = v4();

  rooms
    .insertOne({ id, content: "" })
    .then(() => {
      res.send({ status: "success", result: { id } });
    })
    .catch((err) => {
      if (err instanceof Error) {
        res.send({ status: "error", reason: err.message });
      } else {
        res.send({ status: "error", reason: "Unknown Error." });
      }
    });
});

app.get("/rooms/:id", (req, res) => {
  const id = req.params.id;

  rooms
    .findOne({ id }, { projection: { content: 1 } })
    .then((room) => {
      if (room) {
        res.send({ status: "success", result: { content: room.content } });
      } else {
        throw new Error("Invalid ID.");
      }
    })
    .catch((err) => {
      if (err instanceof Error) {
        res.send({ status: "error", reason: err.message });
      } else {
        res.send({ status: "error", reason: "Unknown Error." });
      }
    });
});
app.put("/rooms/:id/:content", (req, res) => {
  const id = req.params.id;
  const content = req.params.content;

  rooms
    .updateOne({ id }, { $set: { content } })
    .then(() => {
      res.send({ status: "success", result: {} });
    })
    .catch((err) => {
      if (err instanceof Error) {
        res.send({ status: "error", reason: err.message });
      } else {
        res.send({ status: "error", reason: "Unknown Error." });
      }
    });
});
app.delete("/rooms/:id", (req, res) => {
  const id = req.params.id;

  rooms
    .deleteOne({ id })
    .then(() => {
      res.send({ status: "success", result: {} });
    })
    .catch((err) => {
      if (err instanceof Error) {
        res.send({ status: "error", reason: err.message });
      } else {
        res.send({ status: "error", reason: "Unknown Error." });
      }
    });
});

app.listen(8080, "0.0.0.0", () => {
  console.log("Listening on port 8080.");
});
