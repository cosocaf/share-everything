/**
 * @file storage.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * storage.hの実装。
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "storage.h"

#include <pathcch.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

#include <format>

#include "logger.h"

namespace share_everything {
  Storage::Storage() {
    auto file = fopen(getStorageFilePath().c_str(), "r");
    if (!file) {
      LOG_ERROR(_T("Failed to open storage.json"), _T("Storage"));
      return;
    }

    constexpr size_t bufSize = 1024;
    char buf[bufSize];
    rapidjson::FileReadStream stream(file, buf, bufSize);

    rapidjson::Document doc;
    doc.ParseStream(stream);

    fclose(file);

    if (doc.HasParseError()) {
      LOG_ERROR(_T("Failed to parse storage.json"), _T("Storage"));
      return;
    }

    for (const auto& member : doc.GetObject()) {
      auto key   = member.name.GetString();
      auto value = member.value.GetString();
      LOG_INFO(std::format(_T("Found storage key: {}, value: {}"),
                           strToTstr(key),
                           strToTstr(value)),
               _T("Storage"));
      storage[key] = value;
    }
  }
  Storage::~Storage() {
    rapidjson::Document doc;
    doc.SetObject();
    for (const auto& [key, value] : storage) {
      doc.AddMember(rapidjson::StringRef(key.c_str(), key.size()),
                    rapidjson::StringRef(value.c_str(), value.size()),
                    doc.GetAllocator());
    }

    auto file = fopen(getStorageFilePath().c_str(), "w");
    if (!file) {
      LOG_ERROR(_T("Failed to open storage.json"), _T("Storage"));
      return;
    }

    constexpr size_t bufSize = 1024;
    char buf[bufSize];
    rapidjson::FileWriteStream stream(file, buf, bufSize);
    rapidjson::Writer<rapidjson::FileWriteStream> writer(stream);
    doc.Accept(writer);

    fclose(file);

    LOG_INFO(_T("Written to storage."), _T("Storage"));
  }

  std::string& Storage::operator[](const std::string& key) {
    return storage[key];
  }

  std::string Storage::getStorageFilePath() {
    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

#ifdef _UNICODE
    PathCchRemoveFileSpec(exePath, MAX_PATH);
    auto path = wstrToStr(exePath);
#else
    std::wstring wexePath = strToWstr(exePath);
    PathCchRemoveFileSpec(wexePath, wexePath.size());
    auto path = wstrToStr(wexePath);
#endif

    if (path.back() != '\\') path += '\\';
    path += "storage.json";

    LOG_INFO(std::format(_T("Storage File Path: {}"), strToTstr(path)),
             _T("Storage"));

    return path;
  }
} // namespace share_everything