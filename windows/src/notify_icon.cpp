/**
 * @file notify_icon.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-26
 * 
 * notify_icon.hの実装。
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "notify_icon.h"

#include <shellapi.h>

#include "app.h"
#include "resource/resource.h"

namespace share_everything {
  NotifyIcon::~NotifyIcon() noexcept {
    NOTIFYICONDATA nid{};
    nid.cbSize   = sizeof(nid);
    nid.uFlags   = NIF_GUID;
    nid.guidItem = __uuidof(UUID);
    Shell_NotifyIcon(NIM_DELETE, &nid);
  }

  BOOL NotifyIcon::init(HWND hWnd) noexcept {
    NOTIFYICONDATA nid{};
    nid.cbSize           = sizeof(nid);
    nid.hWnd             = hWnd;
    nid.guidItem         = __uuidof(UUID);
    nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.uCallbackMessage = WM_NOTIFY_ICON;
    nid.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    _tcscpy_s(nid.szTip, _T("Share Everything"));
    Shell_NotifyIcon(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
  }

  void NotifyIcon::showContextMenu(App* app, const POINT& pt) {
    HMENU hMenu = LoadMenu(app->getHInst(), MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (!hMenu) return;
    HMENU hSubMenu = GetSubMenu(hMenu, 0);
    if (!hSubMenu) {
      DestroyMenu(hMenu);
      return;
    }

    SetForegroundWindow(app->getHWnd());
    UINT uFlags = TPM_RIGHTBUTTON;
    if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
      uFlags |= TPM_RIGHTALIGN;
    } else {
      uFlags |= TPM_LEFTALIGN;
    }

    TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, app->getHWnd(), nullptr);
    DestroyMenu(hMenu);
  }

  LRESULT CALLBACK NotifyIcon::handleMessage(App* app, WPARAM wp, LPARAM lp) {
    switch (LOWORD(lp)) {
      case NIN_SELECT:
        app->toggleWindowVisibility();
        return 0;
      case WM_CONTEXTMENU:
        showContextMenu(app, { LOWORD(wp), HIWORD(wp) });
        return 0;
    }
    return 0;
  }
} // namespace share_everything