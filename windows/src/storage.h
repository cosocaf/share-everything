/**
 * @file storage.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * 設定情報などを保存する用のユーティリティ。(素直に.iniとかレジストリ使え)
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SHARE_EVERYTHING_STORAGE_H_
#define SHARE_EVERYTHING_STORAGE_H_

#include "pch.h"

#include <map>
#include <string>

namespace share_everything {
  class Storage {
    std::map<std::string, std::string> storage;

  public:
    Storage();
    ~Storage() noexcept;

    std::string& operator[](const std::string& key);

    std::string getStorageFilePath();
  };
} // namespace share_everything

#endif // SHARE_EVERYTHING_STORAGE_H_