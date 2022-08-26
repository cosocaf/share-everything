/**
 * @file notify_icon.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-26
 * 
 * 通知領域のアイコン。地味だけど最重要。
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SHARE_EVERYTHING_NOTIFY_ICON_H_
#define SHARE_EVERYTHING_NOTIFY_ICON_H_

#include "pch.h"

namespace share_everything {
  class App;

  /**
   * @brief 通知領域に存在するアイコンを定義する。
   *
   */
  class NotifyIcon {
    class __declspec(uuid("3afcf918-70ac-4abf-a283-5053a7b0c005")) UUID;

  public:
    static constexpr UINT WM_NOTIFY_ICON = WM_APP + 1;

    ~NotifyIcon() noexcept;

    /**
     * @brief 初期化する。成功したらTRUEを、失敗したらFALSEを返す。
     *
     * @param hWnd ウィンドウハンドル
     * @return BOOL 成功フラグ
     */
    BOOL init(HWND hWnd) noexcept;

    /**
     * @brief 通知領域に届くウィンドウメッセージを処理する。
     *
     * @param app Appインスタンス。
     * @param wp WPARAM
     * @param lp LPARAM
     * @return LRESULT LRESULT
     */
    LRESULT CALLBACK handleMessage(App* app, WPARAM wp, LPARAM lp);

  private:
    /**
     * @brief 通知領域にコンテキストメニューを表示する。
     *
     * @param app Appインスタンス
     * @param pt 表示する座標
     */
    void showContextMenu(App* app, const POINT& pt);
  };
} // namespace share_everything

#endif // SHARE_EVERYTHING_NOTIFY_ICON_H_