/**
 * @file logger.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-26
 * 
 * logger.hの実装。
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "logger.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include <chrono>
#include <format>
#include <iostream>

namespace share_everything {
  Logger::Logger() : consoleEnabled(false) {}
  Logger::~Logger() noexcept {
    if (consoleEnabled) {
      FreeConsole();
      consoleEnabled = false;
    }
  }

  bool Logger::init() {
    FILE* out;
    FILE* in;

    if (!AllocConsole()) {
      return false;
    }
    consoleEnabled = true;

    if (freopen_s(&out, "CONOUT$", "w+", stdout) != 0) {
      return false;
    }
    if (freopen_s(&in, "CONIN$", "r", stdin) != 0) {
      return false;
    }
    _setmode(_fileno(stdout), _O_U8TEXT);

    LOG_INFO(_T("Logger has been initialized."), _T("Logger"));

    return true;
  }

  void Logger::log(tstring_view logLevel,
                   tstring_view message,
                   tstring_view group,
                   tstring_view func,
                   tstring_view file,
                   int line) {
    if (!consoleEnabled) {
      return;
    }
    auto now  = std::chrono::system_clock::now();
    auto text = std::format(_T("{} {} {}/{} {}\n  at {}:{}"),
                            now,
                            func,
                            logLevel,
                            group,
                            message,
                            file,
                            line);
#ifdef _UNICODE
    std::wcout << text << _T('\n');
#else
    std::cout << text << _T('\n');
#endif
  }

  void Logger::debug(tstring_view message,
                     tstring_view group,
                     tstring_view func,
                     tstring_view file,
                     int line) {
    log(_T("D"), message, group, func, file, line);
  }
  void Logger::info(tstring_view message,
                    tstring_view group,
                    tstring_view func,
                    tstring_view file,
                    int line) {
    log(_T("I"), message, group, func, file, line);
  }
  void Logger::warn(tstring_view message,
                    tstring_view group,
                    tstring_view func,
                    tstring_view file,
                    int line) {
    log(_T("W"), message, group, func, file, line);
  }
  void Logger::error(tstring_view message,
                     tstring_view group,
                     tstring_view func,
                     tstring_view file,
                     int line) {
    log(_T("E"), message, group, func, file, line);
  }
  void Logger::fatal(tstring_view message,
                     tstring_view group,
                     tstring_view func,
                     tstring_view file,
                     int line) {
    log(_T("F"), message, group, func, file, line);
  }

  Logger logger;
} // namespace share_everything