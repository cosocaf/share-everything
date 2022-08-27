import { Express, Response } from "express";

/**
 * リクエストを処理するためのクラス。
 * Provider::registerを経由して各リクエストの処理を登録する。
 */
export abstract class Provider {
  /**
   * エラー処理のための汎用メソッド。
   *
   * @param err 起こったエラー
   * @param res レスポンス
   */
  protected handleError(err: unknown, res: Response) {
    console.error(err);
    if (err instanceof Error) {
      res.send({ status: "error", reason: err.message });
    } else {
      res.send({ status: "error", reason: "Unknown Error." });
    }
  }

  /**
   * リクエスト処理を登録する。
   *
   * @param app Expressインスタンス
   */
  abstract register(app: Express): void;
}
