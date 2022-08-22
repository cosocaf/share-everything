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

  std::string tstrToStr(tstring_view tstr);
  tstring strToTstr(std::string_view str);
} // namespace share_everything

#endif // SHARE_EVERYTHING_TCHAR_UTIL_H_