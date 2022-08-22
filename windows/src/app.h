#ifndef SHARE_EVERYTHING_APP_H_
#define SHARE_EVERYTHING_APP_H_

#include "pch.h"

#include <optional>
#include <set>

#include "notify_icon.h"
#include "tchar_util.h"

namespace share_everything {
  class App {
    static constexpr auto className = _T("ShareEverything");
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
    static int CALLBACK enumFontFamExProc(const LOGFONT* enumLogFont,
                                          const TEXTMETRIC* textMetric,
                                          DWORD fontType,
                                          LPARAM lp);

    static constexpr auto windowWidth  = 640;
    static constexpr auto windowHeight = 480;

    // constまみれで草
    static constexpr const TCHAR* const preferredFonts[] = {
      _T("游ゴシック"),
      _T("メイリオ"),
      _T("Segoe UI"),
      _T("Arial"),
    };

    std::set<tstring> availableFonts;

    HINSTANCE hInst;
    HWND hWnd;
    NotifyIcon notifyIcon;
    HWND hIdInput;

    bool isLayoutInited;

    std::optional<tstring> id;

    bool clipboardMonitorEnabled;

  public:
    App(HINSTANCE hInst);
    ~App() noexcept;

    HINSTANCE getHInst() noexcept;
    HWND getHWnd() noexcept;

    int start();
    void setWindowVisibility(bool visible) noexcept;
    void toggleWindowVisibility() noexcept;
    void saveId();
    void deleteId();
    void registerId();

    void uploadText(tstring_view text);

    void copy();
    void paste();

  private:
    void initLayout() noexcept;
    void onUpdateClipboard();
  };
} // namespace share_everything

#endif // SHARE_EVERYTHING_APP_H_