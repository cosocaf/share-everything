import express from "express";
import helmet from "helmet";
import { MongoClient } from "mongodb";

import { v4 } from "uuid";

const app = express();
app.use(helmet());
app.use(express.json());

const client = new MongoClient(
  "mongodb://admin:admin@mongo/share-everything?authSource=admin",
);

try {
  await client.connect();
  console.log("Succesfully connected to mongo.");
} catch (e) {
  console.error(e);
}

const db = client.db();

const rooms = db.collection("rooms");

app.post("/rooms", async (_, res) => {
  // TODO: 認証機能とかあったほうがいいよね
  const id = v4();

  try {
    await rooms.insertOne({ id, content: "" });
    res.send({ status: "success", result: { id } });
  } catch (e) {
    if (e instanceof Error) {
      res.send({ status: "error", reason: e.message });
    } else {
      res.send({ status: "error", reason: "Unknown Error." });
    }
  }
});
app.get("/rooms/:id", async (req, res) => {
  const id = req.params.id;

  try {
    const room = await rooms.findOne({ id }, { content: 1 });
    if (room) {
      res.send({ status: "success", result: { content: room.content } });
    } else {
      res.send({ status: "error", reason: "Invalid ID." });
    }
  } catch (e) {
    if (e instanceof Error) {
      res.send({ status: "error", reason: e.message });
    } else {
      res.send({ status: "error", reason: "Unknown Error." });
    }
  }
});
app.put("/rooms/:id", async (req, res) => {
  const id = req.params.id;
});

app.listen(8080, "0.0.0.0", () => {
  console.log("Listening on port 8080.");
});
