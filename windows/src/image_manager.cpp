#include "image_manager.h"

#include <wrl/client.h>

#include <format>

namespace share_everything {
  bool ImageManager::init() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    auto status
      = Gdiplus::GdiplusStartup(&token, &gdiplusStartupInput, nullptr);
    return status == Gdiplus::Ok;
  }
  ImageManager::~ImageManager() {
    Gdiplus::GdiplusShutdown(token);
  }

  Result<std::vector<char>, tstring> ImageManager::decodeBitmap(
    HBITMAP hBitmap) {
    using Microsoft::WRL::ComPtr;

    auto bitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, nullptr);
    if (bitmap == nullptr) {
      return error(_T("Bitmap::FromHBITMAP failed."));
    }

    ComPtr<IStream> istream = nullptr;
    auto hr                 = CreateStreamOnHGlobal(nullptr, TRUE, &istream);
    if (FAILED(hr)) {
      return error(_T("CreateStreamOnHGlobal failed."));
    }

    auto clsid = getClsid(L"image/png");
    if (!clsid) {
      return error(clsid.err());
    }

    auto status = bitmap->Save(istream.Get(), &clsid.get());
    if (status != Gdiplus::Ok) {
      return error(_T("bitmap->Save failed."));
    }

    STATSTG statstg;
    istream->Stat(&statstg, 0);
    size_t size = statstg.cbSize.QuadPart;

    std::vector<char> buffer;
    buffer.resize(size);

    LARGE_INTEGER dlibMove{};
    istream->Seek(dlibMove, STREAM_SEEK_SET, nullptr);

    istream->Read(buffer.data(), size, nullptr);

    return ok(buffer);
  }

  Result<CLSID, tstring> ImageManager::getClsid(LPCWSTR format) {
    UINT num = 0, size = 0;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) {
      return error(_T("GetImageEncodersSize failed."));
    }

    auto imageCodecInfo = new Gdiplus::ImageCodecInfo[size];
    GetImageEncoders(num, size, imageCodecInfo);

    for (UINT i = 0; i < num; ++i) {
      if (wcscmp(imageCodecInfo[i].MimeType, format) == 0) {
        CLSID clsid = imageCodecInfo[i].Clsid;
        delete[] imageCodecInfo;
        return ok(clsid);
      }
    }

    delete[] imageCodecInfo;
    return error(std::format(_T("MIME {} is not supported."), format));
  }
} // namespace share_everything