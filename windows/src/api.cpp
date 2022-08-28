/**
 * @file api.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * api.hの実装。libcurlに逃げた。
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "api.h"

#include <wincrypt.h>

#include <cassert>
#include <format>

#include "logger.h"

namespace share_everything {
  ApiClient::ApiClient() noexcept {
    curl_global_init(CURL_GLOBAL_DEFAULT);
  }
  ApiClient::~ApiClient() noexcept {
    curl_global_cleanup();
  }

  Result<std::string, std::string> ApiClient::getContent(
    std::string_view roomId) {
    using namespace std::string_literals;

    auto result = fetch(std::format("rooms/{}", roomId), Method::Get);
    if (!result) {
      LOG_ERROR(_T("Failed to fetch."), _T("API"));
      return error(result.err());
    }

    auto checkResponseResult = checkResponse(result.get());
    if (!checkResponseResult) {
      return error(checkResponseResult.err());
    }

    auto doc = std::move(checkResponseResult.get());

    if (!doc["result"]["content"].IsString()) {
      LOG_ERROR(_T("Invalid API result. result.content is not set"), _T("API"));
      return error("正しいAPIのフォーマットではありません。"s);
    }

    return ok(doc["result"]["content"].GetString());
  }
  Result<_, std::string> ApiClient::putTextContent(std::string_view roomId,
                                                   std::string_view content) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    writer.StartObject();
    writer.Key("content");
    writer.String(content.data());
    writer.Key("type");
    writer.String("text");
    writer.Key("format");
    writer.String("raw");
    writer.EndObject();

    auto result
      = fetch(std::format("rooms/{}", roomId), Method::Put, buffer.GetString());
    if (!result) {
      return error(result.err());
    }

    return ok();
  }
  Result<_, std::string> ApiClient::putBinaryContent(
    std::string_view roomId,
    const std::vector<char>& content) {
    DWORD cbBinary;
    CryptBinaryToString((BYTE*)content.data(),
                        content.size(),
                        CRYPT_STRING_BASE64,
                        nullptr,
                        &cbBinary);

    tstring contentBuffer;
    contentBuffer.resize(cbBinary);
    CryptBinaryToString((BYTE*)content.data(),
                        content.size(),
                        CRYPT_STRING_BASE64,
                        contentBuffer.data(),
                        &cbBinary);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    writer.StartObject();
    writer.Key("content");
    writer.String(tstrToStr(contentBuffer).data());
    writer.Key("type");
    writer.String("url");
    writer.Key("format");
    writer.String("base64");
    writer.EndObject();

    auto result
      = fetch(std::format("rooms/{}", roomId), Method::Put, buffer.GetString());
    if (!result) {
      return error(result.err());
    }

    return ok();
  }

  Result<std::string, std::string> ApiClient::createRoom() {
    using namespace std::string_literals;

    auto result = fetch(std::format("rooms"), Method::Post);
    if (!result) {
      return error(result.err());
    }

    auto checkResponseResult = checkResponse(result.get());
    if (!checkResponseResult) {
      return error(checkResponseResult.err());
    }

    auto doc = std::move(checkResponseResult.get());

    if (!doc["result"]["id"].IsString()) {
      LOG_ERROR(_T("Invalid API result. result.id is not set"), _T("API"));
      return error("正しいAPIのフォーマットではありません。"s);
    }

    return ok(doc["result"]["id"].GetString());
  }
  Result<_, std::string> ApiClient::deleteRoom(std::string_view roomId) {
    auto result = fetch(std::format("rooms/{}", roomId), Method::Delete);
    if (!result) {
      return error(result.err());
    }
    return ok();
  }

  Result<std::string, std::string> ApiClient::fetch(
    std::string_view to,
    Method method,
    std::optional<std::string_view> body,
    std::optional<std::string_view> contentType) {
    using namespace std::string_literals;

    auto curl = curl_easy_init();
    if (curl == nullptr) {
      return error("CURLの初期化に失敗しました。"s);
    }

    std::string chunk;

    auto url = std::format("{}://{}:{}/{}", PROTOCOL, HOST_NAME, PORT, to);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

    switch (method) {
      case Method::Get:
        // Do nothing.
        break;
      case Method::Post:
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (body) {
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body->data());
          curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body->size());

          if (contentType) {
            auto slist = curl_slist_append(
              nullptr,
              std::format("Content-Type: {}", contentType.value()).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
          }
        }
        break;
      case Method::Put: {
        assert(body.has_value());

        std::pair<std::string_view*, size_t> pair(&body.value(), 0);
        curl_off_t size = body->size();
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READDATA, &pair);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, size);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
        if (contentType) {
          auto slist = curl_slist_append(
            nullptr,
            std::format("Content-Type: {}", contentType.value()).c_str());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        }
        break;
      }
      case Method::Delete:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
      default:
        curl_easy_cleanup(curl);
        return error("正しいHTTPメソッドではありません。"s);
    }

    auto code = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    if (code != CURLE_OK) {
      return error("通信に失敗しました。"s);
    }

    return ok(chunk);
  }

  Result<rapidjson::Document, std::string> ApiClient::checkResponse(
    std::string_view response) {
    using namespace std::string_literals;

    rapidjson::Document doc;
    doc.Parse(response.data());
    if (doc.HasParseError()) {
      LOG_ERROR(_T("Failed to parse JSON."), _T("API"));
      return error("JSONのパースに失敗しました。"s);
    }

    if (!doc["status"].IsString()) {
      LOG_ERROR(_T("Invalid API result. statis is not set."), _T("API"));
      return error("正しいAPIのフォーマットではありません。"s);
    }

    std::string status = doc["status"].GetString();
    if (status != "success") {
      if (!doc["reason"].IsString()) {
        LOG_ERROR(_T("Invalid API result. reason is not set."), _T("API"));
        return error("正しいAPIのフォーマットではありません。"s);
      }
      LOG_ERROR(std::format(_T("Failed to call API. {}"),
                            strToTstr(doc["reason"].GetString()))
                  .c_str(),
                _T("API"));
      return error("APIの呼び出しに失敗しました。"s);
    }

    if (!doc["result"].IsObject()) {
      LOG_ERROR(_T("Invalid API result. result is not set."), _T("API"));
      return error("正しいAPIのフォーマットではありません。"s);
    }

    return ok(std::move(doc));
  }

  size_t ApiClient::writeCallback(char* buffer,
                                  size_t size,
                                  size_t nmemb,
                                  std::string* chunk) {
    chunk->append(buffer, size * nmemb);
    return size * nmemb;
  }
  size_t ApiClient::readCallback(char* buffer,
                                 size_t size,
                                 size_t nmemb,
                                 std::pair<std::string_view*, size_t>* stream) {
    size_t len = std::min(stream->first->size() - stream->second, size * nmemb);
    memcpy(buffer, stream->first->data() + stream->second, len);
    stream->second += len;
    return len;
  }
} // namespace share_everything