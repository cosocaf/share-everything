import { MongoClient } from "mongodb";

const protocol = process.env.DB_PROTOCOL || "";
const userName = process.env.DB_USER_NAME || "";
const password = process.env.DB_USER_PASSWORD || "";
const hostName = process.env.DB_HOST_NAME || "";
const dbName = process.env.DB_NAME || "";

export type Room = {
  id: string;
  content: string;
};

export const initDatabase = async () => {
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
  const rooms = db.collection<Room>("rooms");

  return { db, rooms };
};
