/**
 * @file tchar_util.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * tchar_utils.hの実装。
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "tchar_util.h"

#include "logger.h"

namespace share_everything {
  std::string tstrToStr(tstring_view tstr) {
#ifdef _UNICODE
    return wstrToStr(tstr);
#else
    return tstr;
#endif
  }
  tstring strToTstr(std::string_view str) {
#ifdef _UNICODE
    return strToWstr(str);
#else
    return str;
#endif
  }
  std::string wstrToStr(std::wstring_view wstr) {
    int bufSize = WideCharToMultiByte(
      CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, 0, nullptr);
    std::string str;
    str.resize(bufSize);
    WideCharToMultiByte(
      CP_UTF8, 0, wstr.data(), wstr.size(), str.data(), str.size(), 0, nullptr);
    return str;
  }
  std::wstring strToWstr(std::string_view str) {
    int bufSize
      = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    std::wstring wstr;
    wstr.resize(bufSize);
    MultiByteToWideChar(
      CP_UTF8, 0, str.data(), str.size(), wstr.data(), wstr.size());
    return wstr;
  }
} // namespace share_everything