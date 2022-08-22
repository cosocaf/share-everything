# Share Everything for Windows

Share EverythingクライアントのWindows実装です。Mac/Linuxの対応は面倒なのでやりません。

## 仕様

- 一度起動すると常駐アプリとして稼働し続ける。
- 右下通知アイコンを右クリックでExitを選択すると終了する。
- 右下通知アイコンを右クリックでSettingを選択すると設定画面が表示される。
  - 設定画面ではIDの登録/削除/作成ができる。
- Ctrl+Alt+Cで選択したテキストをAPIを経由してサーバにアップロードする。
- Ctrl+Alt+Vでサーバにある値をペーストできる。(Chrome拡張はこれができない)
- デバッグのため起動オプションに`--enable-debug`をつけるとログ表示のコンソールが出現する。
  - ログレベルは`Debug` `Info` `Warn` `Error` `Fatal`の5つ。
  - ログ表示時に`D/App`や`E/App`のようにどのグループでどのレベルのログが発生したのかわかるようになっている。(Android Studioのパクリ)

## 開発環境の構築方法

リポジトリはクローンしている前提です。

`CMake`の設定をする。

```sh
cmake -B build
```

`CMake`でビルドする。

```sh
cmake --build build
```

以上です。

一応VSCode用にワークスペースの設定をしているので何もせずとも`F5`で起動できると思います。
