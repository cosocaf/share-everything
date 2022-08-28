/**
 * @file app.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * アプリ本体。
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SHARE_EVERYTHING_APP_H_
#define SHARE_EVERYTHING_APP_H_

#include "pch.h"

#include <optional>
#include <set>

#include "api.h"
#include "image_manager.h"
#include "notify_icon.h"
#include "storage.h"
#include "tchar_util.h"

namespace share_everything {
  /**
   * @brief アプリ全体をコントロールするためのクラス。
   * インスタンスを複数つくるとぶっ壊れると思う。
   *
   */
  class App {
    /**
     * @brief App::hWndのクラス名
     *
     */
    static constexpr auto className = _T("ShareEverything");

    /**
     * @brief 共通ウィンドウプロシージャ
     *
     * @see MSのドキュメント
     *
     * @param hWnd
     * @param msg
     * @param wp
     * @param lp
     * @return LRESULT
     */
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
    /**
     * @brief フォント列挙用のコールバック
     *
     * @see MSのドキュメント
     *
     * @param enumLogFont
     * @param textMetric
     * @param fontType
     * @param lp
     * @return int
     */
    static int CALLBACK enumFontFamExProc(const LOGFONT* enumLogFont,
                                          const TEXTMETRIC* textMetric,
                                          DWORD fontType,
                                          LPARAM lp);

    /**
     * @brief 設定ウィンドウの横幅
     *
     */
    static constexpr auto windowWidth = 640;
    /**
     * @brief 設定ウィンドウの縦幅
     *
     */
    static constexpr auto windowHeight = 480;

    static constexpr auto storageRoomKey = "room_id";

    /**
     * @brief GUIに使用するフォントの優先順列挙。
     *
     */
    static constexpr const TCHAR* const preferredFonts[] = {
      _T("游ゴシック"),
      _T("メイリオ"),
      _T("Segoe UI"),
      _T("Arial"),
    };

    /**
     * @brief GUIに使用するフォント。
     * App::preferredFontsから選ばれる。
     * いずれのフォントも存在しなければ何になるかは不明。
     *
     */
    std::set<tstring> availableFonts;

    /**
     * @brief WinMainの第一引数でもらうやつ。
     *
     */
    HINSTANCE hInst;
    /**
     * @brief 設定画面のウィンドウハンドル。
     *
     */
    HWND hWnd;
    /**
     * @brief 通知領域のやつ。
     * アプリの停止とかはここでやれる。
     *
     */
    NotifyIcon notifyIcon;
    /**
     * @brief ルームID変更用に設定画面に存在するフィールドへのハンドル。
     *
     */
    HWND hIdInput;

    /**
     * @brief 設定画面が初期化されているかのフラグ。
     * trueなら初期化済み。
     *
     */
    bool isLayoutInited;

    /**
     * @brief クリップボードが監視中であるかのフラグ。
     * trueなら監視中。
     *
     */
    bool clipboardMonitorEnabled;

    /**
     * @brief API操作のために裏でごにょごにょやるユーティリティクラス。
     *
     */
    ApiClient apiClient;

    /**
     * @brief 実行外でも情報を保持できるようにする。
     * 現状ルームIDを保存するくらいしか使い道ない。
     *
     */
    Storage storage;

    ImageManager imageManager;

  public:
    /**
     * @brief ただのコンストラクタ。特にいうことなし。
     *
     * @param hInst WinMainの第一引数でもらうやつ。
     */
    App(HINSTANCE hInst);
    /**
     * @brief ただのデストラクタ。特にいうことなし。
     *
     */
    ~App() noexcept;

    /**
     * @brief インスタンスハンドルを取得する。
     *
     * @return HINSTANCE インスタンスハンドル。
     */
    HINSTANCE getHInst() noexcept;
    /**
     * @brief 設定画面のウィンドウハンドルを取得する。
     *
     * @return HWND 設定画面のウィンドウハンドル。
     */
    HWND getHWnd() noexcept;

    /**
     * @brief アプリを開始する。
     *
     * @return int アプリの終了コード。
     */
    int start();
    /**
     * @brief 設定画面の可視性を変更する。
     *
     * @param visible trueなら表示、falseなら非表示。
     */
    void setWindowVisibility(bool visible) noexcept;
    /**
     * @brief 設定画面の可視性を変更する。
     * 表示されているときに実行すれば非表示に、
     * 非表示の時に実行すれば表示される。
     *
     */
    void toggleWindowVisibility() noexcept;
    /**
     * @brief hIdInputに記録されている値を新たなルームIDとして設定する。
     *
     */
    void saveId();
    /**
     * @brief 現在記録しているルームIDを削除する。
     * ルーム自体が削除されるかはAPIが勝手に判断する。多分。
     *
     */
    void deleteId();
    /**
     * @brief ルームIDを新規発行する。
     * どのIDが割り当てられるかはAPIの気持ち次第。
     *
     */
    void registerId();

    /**
     * @brief 現在所属しているルームあてに値の更新命令を発行する。
     * 値の形式はテキスト。
     *
     * @param text 更新する値。
     */
    void uploadText(tstring_view text);

    /**
     * @brief 現在所属しているルームあてに値の更新命令を発行する。
     * 値の形式はBitmap。
     *
     * @param bitmapInfo 更新する値。
     */
    void uploadBitmap(LPBITMAPINFO bitmapInfo, HBITMAP bitmap);

    /**
     * @brief
     * Ctrl+Cをアクティブウィンドウあてに送信してクリップボードにコピーさせる。
     * ただし、即座に反映されるわけではないので別途クリップボードの変更を監視しなければならない。
     *
     */
    void copy();
    /**
     * @brief
     * 現在所属しているルームから値を取り出し、クリップボードにコピーしたのちCtrl+Vをアクティブウィンドウあてに送信してペーストさせる。
     *
     */
    void paste();

  private:
    /**
     * @brief 設定画面を初期化する。Qt使いたい。
     *
     */
    void initLayout() noexcept;
    /**
     * @brief クリップボードが変更されたら呼ばれるコールバック。
     *
     */
    void onUpdateClipboard();
  };
} // namespace share_everything

#endif // SHARE_EVERYTHING_APP_H_