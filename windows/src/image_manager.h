#ifndef SHARE_EVERYTHING_IMAGE_MANAGER_H_
#define SHARE_EVERYTHING_IMAGE_MANAGER_H_

#include "pch.h"

#include <vector>

#include "result.h"
#include "tchar_util.h"

namespace share_everything {
  class ImageManager {
    ULONG_PTR token;

  public:
    bool init();
    ~ImageManager();

    Result<std::vector<char>, tstring> decodeBitmap(HBITMAP hBitmap);

  private:
    Result<CLSID, tstring> getClsid(LPCWSTR format);
  };
} // namespace share_everything

#endif // SHARE_EVERYTHING_IMAGE_MANAGER_H_