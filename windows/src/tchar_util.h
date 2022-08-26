/**
 * @file tchar_util.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * UTF-16は滅びろ。
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SHARE_EVERYTHING_TCHAR_UTIL_H_
#define SHARE_EVERYTHING_TCHAR_UTIL_H_

#include "pch.h"

#include <string>
#include <string_view>

namespace share_everything {
  using tstring      = std::basic_string<TCHAR>;
  using tstring_view = std::basic_string_view<TCHAR>;
#ifdef _UNICODE
#define to_tstring std::to_wstring
#else
#define to_tstring std::to_string
#endif

  /**
   * @brief TCHARをcharに変換する。
   *
   * @param tstr
   * @return std::string
   */
  std::string tstrToStr(tstring_view tstr);
  /**
   * @brief charをTCHARに変換する。
   *
   * @param str
   * @return tstring
   */
  tstring strToTstr(std::string_view str);
  /**
   * @brief wchar_tをcharに変換する。
   *
   * @param wstr
   * @return std::string
   */
  std::string wstrToStr(std::wstring_view wstr);
  /**
   * @brief charをwchar_tに変換する。
   *
   * @param str
   * @return std::wstring
   */
  std::wstring strToWstr(std::string_view str);
} // namespace share_everything

#endif // SHARE_EVERYTHING_TCHAR_UTIL_H_