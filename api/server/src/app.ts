import express from "express";
import helmet from "helmet";
import cors from "cors";
import createError from "http-errors";

import { initDatabase } from "./db.js";
import { AWSClient } from "./aws.js";
import { RoomsProvider } from "./rooms_provider.js";
import { ContentsProvider } from "./contents_provider.js";

const app = express();
app.use(helmet());
app.use(cors());
app.use(express.json({ limit: "1024MB" }));
app.use(express.urlencoded({ extended: true, limit: "1024MB" }));

const { rooms } = await initDatabase();
const awsClient = new AWSClient();

const roomsProvider = new RoomsProvider(rooms, awsClient);
roomsProvider.register(app);

const contentsProvider = new ContentsProvider(rooms, awsClient);
contentsProvider.register(app);

app.use((_req, _res, next) => {
  return next(createError(404));
});

app.listen(8080, "0.0.0.0", () => {
  console.log("Listening on port 8080.");
});
