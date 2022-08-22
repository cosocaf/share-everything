#include "tchar_util.h"

#include "logger.h"

namespace share_everything {
  std::string tstrToStr(tstring_view tstr) {
#ifdef _UNICODE
    int bufSize = WideCharToMultiByte(
      CP_UTF8, 0, tstr.data(), tstr.size(), nullptr, 0, 0, nullptr);
    std::string str;
    str.resize(bufSize);
    WideCharToMultiByte(
      CP_UTF8, 0, tstr.data(), tstr.size(), str.data(), str.size(), 0, nullptr);
    return str;
#else
    return tstr;
#endif
  }
  tstring strToTstr(std::string_view str) {
#ifdef _UNICODE
    int bufSize
      = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    tstring tstr;
    tstr.resize(bufSize);
    MultiByteToWideChar(
      CP_UTF8, 0, str.data(), str.size(), tstr.data(), tstr.size());
    return tstr;
#else
    return str;
#endif
  }
} // namespace share_everything