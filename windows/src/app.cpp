#include "app.h"

#include <Richedit.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <format>

#include "fetch.h"
#include "logger.h"
#include "resource/resource.h"

#define IDB_SAVE_BUTTON 500
#define IDB_DELETE_BUTTON 501
#define IDB_REGISTER_BUTTON 502
#define IDH_COPY 1001
#define IDH_PASTE 1002

namespace share_everything {
#ifdef _UNICODE
  using tValue        = rapidjson::GenericValue<rapidjson::UTF16<>>;
  using tDocument     = rapidjson::GenericDocument<rapidjson::UTF16<>>;
  using tStringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF16<>>;
  using tWriter
    = rapidjson::Writer<tStringBuffer, rapidjson::UTF16<>, rapidjson::UTF16<>>;
#else
  using tValue        = rapidjson::Value;
  using tDocument     = rapidjson::Document;
  using tStringBuffer = rapidjson::StringBuffer;
  using tWriter       = rapidjson::Writer<tStringBuffer>;
#endif

  App::App(HINSTANCE hInst)
    : hInst(hInst),
      hWnd(nullptr),
      isLayoutInited(false),
      clipboardMonitorEnabled(false),
      hIdInput(nullptr) {
    LOG_INFO(_T("Creating App instance..."), _T("App"));
    WNDCLASSEX wndcls{};
    wndcls.cbSize        = sizeof(wndcls);
    wndcls.style         = CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc   = wndProc;
    wndcls.hInstance     = hInst;
    wndcls.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wndcls.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wndcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndcls.lpszClassName = className;

    if (!RegisterClassEx(&wndcls)) {
      LOG_ERROR(std::format(_T("Failed to register class: {}."), className),
                _T("App"));
      MessageBox(nullptr,
                 _T("ウィンドウクラスの登録に失敗しました。"),
                 _T("FATAL ERROR"),
                 MB_ICONERROR);
      exit(GetLastError());
    }

    auto cx = GetSystemMetrics(SM_CXSCREEN);
    auto cy = GetSystemMetrics(SM_CYSCREEN);

    hWnd
      = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE | WS_EX_COMPOSITED,
                       className,
                       _T("Share Everything"),
                       WS_CAPTION | WS_BORDER | WS_SYSMENU,
                       (cx - windowWidth) / 2,
                       (cy - windowHeight) / 2,
                       windowWidth,
                       windowHeight,
                       nullptr,
                       nullptr,
                       hInst,
                       this);

    if (!hWnd) {
      LOG_ERROR(_T("Failed to create hwnd."), _T("App"));
      MessageBox(NULL,
                 _T("ウィンドウの作成に失敗しました。"),
                 _T("FATAL ERROR"),
                 MB_ICONERROR);
      exit(GetLastError());
    }

    if (!AddClipboardFormatListener(hWnd)) {
      LOG_ERROR(_T("Failed to add clipboard format listener."), _T("App"));
      MessageBox(NULL,
                 _T("クリップボード監視機能の初期化に失敗しました。"),
                 _T("FATAL ERROR"),
                 MB_ICONERROR);
      exit(GetLastError());
    }

    RegisterHotKey(hWnd, IDH_COPY, MOD_CONTROL | MOD_ALT, 'C');
    RegisterHotKey(hWnd, IDH_PASTE, MOD_CONTROL | MOD_ALT, 'V');

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    LOG_INFO(_T("Created App instance."), _T("App"));
  }
  App::~App() {
    if (id) {
      deleteId();
    }
    if (hWnd != nullptr) {
      UnregisterHotKey(hWnd, IDH_COPY);
      UnregisterHotKey(hWnd, IDH_PASTE);
      DestroyWindow(hWnd);
      hWnd = nullptr;
    }
  }

  HINSTANCE App::getHInst() noexcept {
    return hInst;
  }
  HWND App::getHWnd() noexcept {
    return hWnd;
  }

  int App::start() {
    LOG_INFO(_T("Start a message loop."), _T("App"));
    MSG msg;
    while (auto ret = GetMessage(&msg, NULL, 0, 0)) {
      if (ret == -1) {
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
  }
  void App::setWindowVisibility(bool visible) noexcept {
    if (visible) {
      ShowWindow(hWnd, SW_SHOW);
      initLayout();
      SetForegroundWindow(hWnd);
    } else {
      ShowWindow(hWnd, SW_HIDE);
    }
  }
  void App::toggleWindowVisibility() noexcept {
    setWindowVisibility(!IsWindowVisible(hWnd));
  }
  void App::saveId() {
    TCHAR id[128];
    GetWindowText(hIdInput, id, 128);

    if (_tcslen(id) != 6) {
      MessageBox(hWnd, _T("IDは六桁の英数字です。"), _T("Error"), MB_ICONERROR);
      return;
    }

    this->id = id;
    LOG_INFO(_T("ID successfully saved."), _T("App"));
    MessageBox(hWnd, _T("IDを保存しました。"), _T("成功"), MB_ICONINFORMATION);
  }
  void App::deleteId() {
    if (!id) return;
    auto response = fetch(HttpRequest(
      HttpMethod::Delete, std::format(_T("/rooms/{}"), id.value()), {}));
    if (!response) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (response->status != 200) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    tDocument doc;
    doc.Parse(response->body.data());
    if (doc.HasParseError()) {
      MessageBox(
        hWnd, _T("JSONの解析に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("status")].IsString()) {
      MessageBox(hWnd, _T("削除に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    LPCTSTR status = doc[_T("status")].GetString();
    if (_tcscmp(status, _T("success")) != 0) {
      MessageBox(hWnd, _T("削除に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    LOG_INFO(_T("ID successfully deleted."), _T("App"));
    MessageBox(hWnd, _T("IDを削除しました。"), _T("成功"), MB_ICONINFORMATION);
  }
  void App::registerId() {
    auto response = fetch(HttpRequest(HttpMethod::Post, _T("/rooms"), {}));
    if (!response) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (response->status != 200) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    tDocument doc;
    doc.Parse(response->body.data());
    if (doc.HasParseError()) {
      MessageBox(
        hWnd, _T("JSONの解析に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("status")].IsString()) {
      MessageBox(hWnd, _T("登録に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    LPCTSTR status = doc[_T("status")].GetString();
    if (_tcscmp(status, _T("success")) != 0) {
      MessageBox(hWnd, _T("登録に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("result")].IsObject()) {
      MessageBox(hWnd, _T("登録に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("result")][_T("id")].IsString()) {
      MessageBox(hWnd, _T("登録に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    tstring id = doc[_T("result")][_T("id")].GetString();

    if (!OpenClipboard(hWnd)) {
      MessageBox(hWnd, _T("登録に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }
    EmptyClipboard();

    auto hMem = GlobalAlloc(GHND | GMEM_SHARE, sizeof(TCHAR) * (id.size() + 1));
    auto clipboard = reinterpret_cast<LPTSTR>(GlobalLock(hMem));
    _tcscpy(clipboard, id.c_str());
    GlobalUnlock(hMem);
#ifdef _UNICODE
    SetClipboardData(CF_UNICODETEXT, hMem);
#else
    SetClipboardData(CF_TEXT, hMem);
#endif
    CloseClipboard();

    this->id = id;

    LOG_INFO(_T("ID successfully registered."), _T("App"));
    LOG_INFO(_T("ID copied to clipboard."), _T("App"));

    MessageBox(
      hWnd,
      std::format(_T("ID: {}\nクリップボードにコピーしました。"), id).c_str(),
      _T("成功"),
      MB_ICONINFORMATION);

    SetWindowText(hIdInput, id.c_str());

    InvalidateRect(hWnd, nullptr, TRUE);
    UpdateWindow(hWnd);
  }

  void App::uploadText(tstring_view text) {
    if (!id) {
      LOG_INFO(_T("No ID registered."), _T("App"));
      return;
    }

    tStringBuffer buffer;
    tWriter writer(buffer);
    writer.StartObject();
    writer.Key(_T("content"));
    writer.String(text.data());
    writer.EndObject();

    auto response
      = fetch(HttpRequest(HttpMethod::Put,
                          std::format(_T("/rooms/{}"), id.value()),
                          { { _T("Content-Type"), _T("application/json") } },
                          buffer.GetString()));
    if (!response) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (response->status != 200) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    tDocument doc;
    doc.Parse(response->body.data());
    if (doc.HasParseError()) {
      MessageBox(
        hWnd, _T("JSONの解析に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("status")].IsString()) {
      MessageBox(hWnd, _T("登録に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    LPCTSTR status = doc[_T("status")].GetString();
    if (_tcscmp(status, _T("success")) != 0) {
      MessageBox(hWnd, _T("登録に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    LOG_INFO(_T("Text successfully uploaded."), _T("App"));
  }

  void App::copy() {
    if (!id) {
      LOG_INFO(_T("No ID registered."), _T("App"));
      setWindowVisibility(true);
      MessageBox(hWnd,
                 _T("IDが登録されていません。IDを登録してください。"),
                 _T("Error"),
                 MB_ICONERROR);
      return;
    }

    GUITHREADINFO guiThreadInfo;
    guiThreadInfo.cbSize = sizeof(guiThreadInfo);
    if (!GetGUIThreadInfo(NULL, &guiThreadInfo)) {
      LOG_ERROR(_T("Failed to get gui thread info."), _T("App"));
      return;
    }

    constexpr int inputSize = 6;
    INPUT inputs[inputSize];
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].ki.wVk     = VK_MENU;
    inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[1].ki.wVk = VK_CONTROL;

    inputs[2].ki.wVk = 'C';

    inputs[3].ki.wVk     = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[4].ki.wVk     = 'C';
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[5].ki.wVk = VK_MENU;

    for (auto& input : inputs) {
      input.type     = INPUT_KEYBOARD;
      input.ki.wScan = MapVirtualKey(input.ki.wVk, MAPVK_VK_TO_VSC);
    }

    if (SendInput(inputSize, inputs, sizeof(INPUT)) != inputSize) {
      LOG_ERROR(_T("Failed to send input."), _T("App"));
      return;
    }

    clipboardMonitorEnabled = true;
  }
  void App::paste() {
    if (!id) {
      LOG_INFO(_T("No ID registered."), _T("App"));
      setWindowVisibility(true);
      MessageBox(hWnd,
                 _T("IDが登録されていません。IDを登録してください。"),
                 _T("Error"),
                 MB_ICONERROR);
      return;
    }

    GUITHREADINFO guiThreadInfo;
    guiThreadInfo.cbSize = sizeof(guiThreadInfo);
    if (!GetGUIThreadInfo(NULL, &guiThreadInfo)) {
      LOG_ERROR(_T("Failed to get gui thread info."), _T("App"));
      return;
    }

    auto response = fetch(HttpRequest(
      HttpMethod::Get, std::format(_T("/rooms/{}"), id.value()), {}));
    if (!response) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (response->status != 200) {
      MessageBox(hWnd, _T("通信に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    tDocument doc;
    doc.Parse(response->body.data());
    if (doc.HasParseError()) {
      MessageBox(
        hWnd, _T("JSONの解析に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("status")].IsString()) {
      MessageBox(
        hWnd, _T("情報の取得に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    LPCTSTR status = doc[_T("status")].GetString();
    if (_tcscmp(status, _T("success")) != 0) {
      MessageBox(
        hWnd, _T("情報の取得に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("result")].IsObject()) {
      MessageBox(
        hWnd, _T("情報の取得に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    if (!doc[_T("result")][_T("content")].IsString()) {
      MessageBox(
        hWnd, _T("情報の取得に失敗しました。"), _T("ERROR"), MB_ICONERROR);
      return;
    }

    tstring content = doc[_T("result")][_T("content")].GetString();

    if (!OpenClipboard(hWnd)) {
      MessageBox(hWnd,
                 _T("クリップボード操作に失敗しました。"),
                 _T("ERROR"),
                 MB_ICONERROR);
      return;
    }
    EmptyClipboard();

    auto hMem
      = GlobalAlloc(GHND | GMEM_SHARE, sizeof(TCHAR) * (content.size() + 1));
    auto clipboard = reinterpret_cast<LPTSTR>(GlobalLock(hMem));
    _tcscpy(clipboard, content.c_str());
    GlobalUnlock(hMem);
#ifdef _UNICODE
    SetClipboardData(CF_UNICODETEXT, hMem);
#else
    SetClipboardData(CF_TEXT, hMem);
#endif
    CloseClipboard();

    LOG_INFO(_T("Content copied to clipboard."), _T("App"));

    constexpr int inputSize = 6;
    INPUT inputs[inputSize];
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].ki.wVk     = VK_MENU;
    inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[1].ki.wVk = VK_CONTROL;

    inputs[2].ki.wVk = 'V';

    inputs[3].ki.wVk     = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[4].ki.wVk     = 'V';
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[5].ki.wVk = VK_MENU;

    for (auto& input : inputs) {
      input.type     = INPUT_KEYBOARD;
      input.ki.wScan = MapVirtualKey(input.ki.wVk, MAPVK_VK_TO_VSC);
    }

    if (SendInput(inputSize, inputs, sizeof(INPUT)) != inputSize) {
      LOG_ERROR(_T("Failed to send input."), _T("App"));
      return;
    }

    LOG_INFO(_T("Successfully pasted from clipboard."), _T("App"));
  }

  // こういうの書いてるとQt使いたくなる
  void App::initLayout() noexcept {
    if (isLayoutInited) return;

    const auto error = [](tstring_view msg) {
      LOG_ERROR(msg, _T("App"));
      MessageBox(NULL,
                 _T("レイアウトの初期化に失敗しました。"),
                 _T("FATAL ERROR"),
                 MB_ICONERROR);
      PostQuitMessage(GetLastError());
    };

    HDC hdc = GetDC(hWnd);
    LOGFONT logFont{ .lfCharSet        = DEFAULT_CHARSET,
                     .lfPitchAndFamily = FF_DONTCARE };
    EnumFontFamiliesEx(hdc, &logFont, enumFontFamExProc, (LPARAM)this, 0);
    ReleaseDC(hWnd, hdc);

    logFont.lfHeight         = 0;
    logFont.lfWidth          = 0;
    logFont.lfEscapement     = 0;
    logFont.lfOrientation    = 0;
    logFont.lfWeight         = 0;
    logFont.lfItalic         = 0;
    logFont.lfUnderline      = 0;
    logFont.lfStrikeOut      = 0;
    logFont.lfCharSet        = DEFAULT_CHARSET;
    logFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    logFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    logFont.lfQuality        = PROOF_QUALITY;
    logFont.lfPitchAndFamily = FF_DONTCARE;

    for (const auto preferredFont : preferredFonts) {
      if (availableFonts.find(preferredFont) != availableFonts.end()) {
        _tcscpy_s(logFont.lfFaceName, preferredFont);
        break;
      }

      bool found = false;
      for (const auto availableFont : availableFonts) {
        if (availableFont.starts_with(preferredFont)) {
          _tcscpy_s(logFont.lfFaceName, availableFont.c_str());
        }
      }
      if (found) break;
    }

    LOG_INFO(std::format(_T("Font found: {}."), logFont.lfFaceName), _T("App"));

    HWND title = CreateWindow(_T("STATIC"),
                              _T("Share Everythingの設定"),
                              WS_CHILD | WS_VISIBLE | SS_CENTER,
                              0,
                              40,
                              windowWidth,
                              40,
                              hWnd,
                              0,
                              hInst,
                              nullptr);
    if (title == nullptr) {
      error(_T("Failed to create title label."));
      return;
    }
    logFont.lfHeight = 40;
    logFont.lfWeight = FW_BOLD;
    HFONT hTitleFont = CreateFontIndirect(&logFont);
    if (hTitleFont == nullptr) {
      error(_T("Failed to create title label font."));
      return;
    }
    SendMessage(title, WM_SETFONT, (WPARAM)hTitleFont, 0);

    int inputY = 130;

    int labelX = 150;
    int labelW = 50;
    int inputX = labelX + labelW;
    int inputW = windowWidth - labelX - inputX;

    logFont.lfHeight = 26;
    logFont.lfWeight = FW_NORMAL;
    HFONT hInputFont = CreateFontIndirect(&logFont);
    if (hInputFont == nullptr) {
      error(_T("Failed to create input font."));
      return;
    }

    HWND hIdLabel = CreateWindow(_T("STATIC"),
                                 _T("ID: "),
                                 WS_CHILD | WS_VISIBLE,
                                 labelX,
                                 inputY,
                                 labelW,
                                 30,
                                 hWnd,
                                 nullptr,
                                 hInst,
                                 nullptr);
    if (hIdLabel == nullptr) {
      error(_T("Failed to create id label."));
      return;
    }
    SendMessage(hIdLabel, WM_SETFONT, (WPARAM)hInputFont, 0);

    hIdInput = CreateWindow(_T("EDIT"),
                            _T(""),
                            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
                            inputX,
                            inputY,
                            inputW,
                            30,
                            hWnd,
                            nullptr,
                            hInst,
                            nullptr);
    if (hIdInput == nullptr) {
      error(_T("Failed to create id edit."));
      return;
    }

    SendMessage(hIdInput, WM_SETFONT, (WPARAM)hInputFont, 0);

    int buttonY = 210;
    int buttonH = 40;

    int buttonX = 150;
    int buttonW = 150;

    HWND hSaveButton = CreateWindow(_T("BUTTON"),
                                    _T("保存"),
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    buttonX,
                                    buttonY,
                                    buttonW,
                                    buttonH,
                                    hWnd,
                                    (HMENU)IDB_SAVE_BUTTON,
                                    hInst,
                                    nullptr);
    if (hSaveButton == nullptr) {
      error(_T("Failed to create save button."));
      return;
    }

    HWND hDeleteButton = CreateWindow(_T("BUTTON"),
                                      _T("削除"),
                                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      windowWidth - buttonX - buttonW,
                                      buttonY,
                                      buttonW,
                                      buttonH,
                                      hWnd,
                                      (HMENU)IDB_DELETE_BUTTON,
                                      hInst,
                                      nullptr);
    if (hDeleteButton == nullptr) {
      error(_T("Failed to create delete button."));
      return;
    }

    logFont.lfWeight  = FW_BOLD;
    HFONT hButtonFont = CreateFontIndirect(&logFont);
    if (hButtonFont == nullptr) {
      error(_T("Failed to create button font."));
      return;
    }

    SendMessage(hSaveButton, WM_SETFONT, (WPARAM)hButtonFont, 0);
    SendMessage(hDeleteButton, WM_SETFONT, (WPARAM)hButtonFont, 0);

    HWND hRegisterLabel = CreateWindow(_T("STATIC"),
                                       _T("もしくは"),
                                       WS_CHILD | WS_VISIBLE | SS_CENTER,
                                       (windowWidth - 100) / 2,
                                       290,
                                       100,
                                       30,
                                       hWnd,
                                       nullptr,
                                       hInst,
                                       nullptr);
    if (hRegisterLabel == nullptr) {
      error(_T("Failed to create register label."));
      return;
    }
    SendMessage(hRegisterLabel, WM_SETFONT, (WPARAM)hInputFont, 0);

    HWND hRegisterButton = CreateWindow(_T("BUTTON"),
                                        _T("新規作成"),
                                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        (windowWidth - buttonW) / 2,
                                        340,
                                        buttonW,
                                        buttonH,
                                        hWnd,
                                        (HMENU)IDB_REGISTER_BUTTON,
                                        hInst,
                                        nullptr);
    if (hRegisterButton == nullptr) {
      error(_T("Failed to create register button."));
      return;
    }
    SendMessage(hRegisterButton, WM_SETFONT, (WPARAM)hButtonFont, 0);

    isLayoutInited = true;
  }

  void App::onUpdateClipboard() {
    if (clipboardMonitorEnabled) {
      clipboardMonitorEnabled = false;

      if (!OpenClipboard(nullptr)) {
        LOG_ERROR(_T("Failed to open clipboard."), _T("App"));
        return;
      }

#ifdef _UNICODE
      auto hMem = GetClipboardData(CF_UNICODETEXT);
#else
      auto hMem = GetClipboardData(CF_TEXT);
#endif
      tstring text = (LPTSTR)GlobalLock(hMem);

      GlobalUnlock(hMem);
      CloseClipboard();

      LOG_INFO(std::format(
                 _T("Clipboard contents were successfully copied: {}"), text),
               _T("App"));

      uploadText(text);
    }
  }

  LRESULT CALLBACK App::wndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    App* app = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg) {
      case NotifyIcon::WM_NOTIFY_ICON:
        return app->notifyIcon.handleMessage(app, wp, lp);
      case WM_CLIPBOARDUPDATE:
        app->onUpdateClipboard();
        return 0;
      case WM_HOTKEY:
        switch (wp) {
          case IDH_COPY:
            app->copy();
            return 0;
          case IDH_PASTE:
            app->paste();
            return 0;
        }
        break;
      case WM_COMMAND:
        switch (LOWORD(wp)) {
          case IDB_SAVE_BUTTON:
            app->saveId();
            return 0;
          case IDB_DELETE_BUTTON:
            app->deleteId();
            return 0;
          case IDB_REGISTER_BUTTON:
            app->registerId();
            return 0;
          case IDM_SETTING:
            ShowWindow(hWnd, SW_SHOW);
            app->initLayout();
            return 0;
          case IDM_EXIT:
            DestroyWindow(hWnd);
            PostQuitMessage(0);
            return 0;
        }
        break;
      case WM_CTLCOLORSTATIC:
        return (LRESULT)GetStockObject(WHITE_BRUSH);
      case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        return 0;
      case WM_CREATE:
        app = reinterpret_cast<App*>(
          reinterpret_cast<LPCREATESTRUCT>(lp)->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        if (!app->notifyIcon.init(hWnd)) {
          MessageBox(NULL,
                     _T("タスクバーの初期化に失敗しました。"),
                     _T("FATAL ERROR"),
                     MB_ICONERROR);
          PostQuitMessage(-1);
          return 0;
        }
        return 0;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
  }
  int CALLBACK
  App::enumFontFamExProc(const LOGFONT* enumLogFont,
                         [[maybe_unused]] const TEXTMETRIC* textMetric,
                         [[maybe_unused]] DWORD fontType,
                         LPARAM lp) {
    auto app = reinterpret_cast<App*>(lp);
    app->availableFonts.insert(enumLogFont->lfFaceName);
    return TRUE;
  }
} // namespace share_everything