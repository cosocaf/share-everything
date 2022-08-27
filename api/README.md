# Share Everything API

Share Everythingのバックエンドです。

nginx + mongo + node + typescript + dockerで開発しています。

堅牢性よりも可用性を重視するためにapache/mysqlではなくnginx/mongoの組み合わせでサーバを構築します。

node/tsを選択したのはただの趣味です。

APIは基本的にRESTfulをベースとして設計。
ただしRESTをちゃんと理解しているわけではないのでこれであっているかは謎です。

現状の実装は全てをテキストとして保持します。
画像もbase64でエンコードしてdataurl形式で持っている状態です。
将来的には画像は別で管理したいところ。S3とか。

MongoDBはAtlas、APIサーバはAWSで本番運営しています。
運営しているドメインは`share-everything-api.cosocaf.com`です。
TLS対応しているので<https://share-everything-api.cosocaf.com>へアクセスできます。

## API概要

このサービスは複数の集団がそれぞれ独立した空間を持ち、その空間内で特定の値を共有するものです。

それぞれの共有空間を`room`と(勝手に)呼び、それを操作するAPIを定義します。

### GET /rooms/:id

`:id`で示される共有空間に格納されている値を取り出す。

APIから返される値のフォーマットは次の通り。

```ts
{
  "status": "success" | "error",
  // もしstatusがsuccessならば
  "result": {
    "content": "<共有空間に格納されている値>",
  },
  // もしstatusがerrorならば
  "reason": "<エラーになった原因>",
}
```

### POST /rooms

共有空間を新規作成する。

APIから返される値のフォーマットは次の通り。

```ts
{
  "status": "success" | "error",
  // もしstatusがsuccessならば
  "result": {
    "id": "<共有空間のID>"
  },
  // もしstatusがerrorならば
  "reason": "<エラーになった原因>",
}
```

### PUT /rooms/:id

`:id`で示される共有空間に値を格納する。

APIから返される値のフォーマットは次の通り。

```ts
{
  "status": "success" | "error",
  // もしstatusがsuccessならば
  "result": {},
  // もしstatusがerrorならば
  "reason": "<エラーになった原因>",
}
```

また、送信者はリクエストのボディに次のフォーマットのデータを格納しなければならない。

HTTPヘッダに`Content-Type: application/json`を指定すること。

```ts
{
  "content": "<格納する値>"
}
```

### DELETE /rooms/:id

`:id`で示される共有空間を削除する。

APIから返される値のフォーマットは次の通り。

```ts
{
  "status": "success" | "error",
  // もしstatusがsuccessならば
  "result": {},
  // もしstatusがerrorならば
  "reason": "<エラーになった原因>",
}
```

## 開発環境の構築方法

リポジトリはクローンしている前提です。

- `server`へ移動します。

```sh
cd ./server
```

- `npm`を設定し、ビルドします。

```sh
npm install
npm run build
```

- Dockerを起動します。

```sh
cd ../
docker-compose up --build
```

以上です。まとめると次のコマンドになります。

```sh
cd ./server
npm install
npm run build
cd ../
docker-compose up --build
```

これで動くはず。多分。

## TODO

ファイルをアップロードできるようにする。
ただしJSON形式でアップロードさせたいのでBase64とかにエンコードしてアップロードする。
`PUT`のbodyを拡張して以下の形にする。

```ts
{
  "content": "<格納する値>",
  "type": "text" | "base64",
  // mimeの一例、任意のmimeを受け入れる予定
  "mime": "text/plane" | "image/png" | "audio/mpeg" | "font/ttf" | "application/pdf"
}
```

ただしmimeで嘘ついている可能性が十分あるのでセキュリティリスクになりうる。
