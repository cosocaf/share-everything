import express from "express";
import helmet from "helmet";
import { MongoClient } from "mongodb";

const app = express();
app.use(helmet());

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

app.listen(8080, "0.0.0.0", () => {
  console.log("Listening on port 8080.");
});
