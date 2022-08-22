#include "pch.h"

#include <vector>

#include "app.h"
#include "logger.h"

namespace share_everything {
  tstring_view trimWhiteSpace(tstring_view str) {
    auto pos = str.find_first_not_of(_T(" \t\n\r"));
    if (pos != str.npos) {
      str = str.substr(pos);
    }
    pos = str.find_last_not_of(_T(" \t\n\r"));
    if (pos != str.npos) {
      str = str.substr(0, pos + 1);
    }
    return str;
  }

  std::vector<tstring> parseCmdLine(LPCTSTR lpCmdLine) {
    std::vector<tstring> result;
    auto cmdLine = trimWhiteSpace(lpCmdLine);
    while (!cmdLine.empty()) {
      auto pos = cmdLine.find(_T(" "));
      result.emplace_back(cmdLine.substr(0, pos));
      if (pos == cmdLine.npos) {
        break;
      }
      cmdLine = trimWhiteSpace(cmdLine.substr(pos + 1));
    }
    return result;
  }
} // namespace share_everything

int WINAPI _tWinMain(HINSTANCE hInst,
                     [[maybe_unused]] HINSTANCE hPrev,
                     LPTSTR lpCmdLine,
                     [[maybe_unused]] int nCmdShow) {
  auto args = share_everything::parseCmdLine(lpCmdLine);
  if (std::find(args.begin(), args.end(), _T("--enable-debug")) != args.end()) {
    if (!share_everything::logger.init()) {
      return -1;
    }
  }

  LOG_INFO(_T("Starting application..."), _T("System"));
  share_everything::App app(hInst);
  const auto result = app.start();
  LOG_INFO(_T("Quit the application"), _T("System"));
  return result;
}