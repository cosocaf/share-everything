import { S3Client, GetObjectCommand } from "@aws-sdk/client-s3";
import { Upload } from "@aws-sdk/lib-storage";
import { Blob } from "buffer";
import { Readable } from "stream";
import { ReadableStream } from "stream/web";

const accessKeyId = process.env.AWS_ACCESS_KEY || "";
const secretAccessKey = process.env.AWS_SECRET_KEY || "";
const region = process.env.AWS_S3_REGION || "";
const bucketName = process.env.AWS_S3_BUCKET_NAME || "";

export class AWSClient {
  private s3Client: S3Client;
  constructor() {
    console.log(
      `Connecting to AWS S3: ${JSON.stringify({ region, bucketName })}`,
    );
    this.s3Client = new S3Client({
      credentials: { accessKeyId, secretAccessKey },
      region,
    });
    console.log("Succesfully connected to AWS S3");
  }

  async upload(key: string, body: Readable | Buffer) {
    console.log("Uploading to AWS S3");
    const upload = new Upload({
      client: this.s3Client,
      params: {
        Bucket: bucketName,
        Key: key,
        Body: body,
      },
    });
    upload.on("httpUploadProgress", (progress) => {
      console.log(progress);
    });
    await upload.done();
    console.log("Succesfully uploaded to AWS S3");
  }
  async download(key: string): Promise<ReadableStream> {
    const response = await this.s3Client.send(
      new GetObjectCommand({
        Bucket: bucketName,
        Key: key,
      }),
    );
    if (response.Body instanceof Readable) {
      const readable = response.Body;
      return new ReadableStream({
        start: (controller) => {
          console.log("start");
          readable.on("data", (chunk) => {
            console.log("data: ", chunk);
            if (chunk instanceof Buffer) {
              controller.enqueue(chunk);
            }
          });
          readable.on("end", () => {
            console.log("end");
            controller.close();
          });
        },
        type: "bytes",
      });
    } else if (response.Body instanceof ReadableStream) {
      return response.Body;
    } else if (response.Body instanceof Blob) {
      const stream = response.Body.stream();
      if (stream instanceof ReadableStream) {
        return stream;
      }
    }

    throw new Error("Invalid instance type.");
  }
}
