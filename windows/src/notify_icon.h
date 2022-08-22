#ifndef SHARE_EVERYTHING_NOTIFY_ICON_H_
#define SHARE_EVERYTHING_NOTIFY_ICON_H_

#include "pch.h"

namespace share_everything {
  class App;
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

    LRESULT CALLBACK handleMessage(App* app, WPARAM wp, LPARAM lp);

  private:
    void showContextMenu(App* app, const POINT& pt);
  };
} // namespace share_everything

#endif // SHARE_EVERYTHING_NOTIFY_ICON_H_