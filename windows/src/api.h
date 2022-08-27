/**
 * @file api.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * APIへの橋渡し。
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SHARE_EVERYTHING_API_H_
#define SHARE_EVERYTHING_API_H_

#include "pch.h"
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <optional>
#include <string>

#include "result.h"

namespace share_everything {
  /**
   * @brief APIとの橋渡しを担うクラス。
   * 一度WinSock2を使って実装したけどTLSでめんどくさくなって
   * libcurl使う実装に切り替えました。
   *
   */
  class ApiClient {
    /**
     * @brief APIのプロトコル。ローカルでテストするときはhttpにする。
     *
     */
    static constexpr auto PROTOCOL = "https";
    // static constexpr auto PROTOCOL = "http";
    /**
     * @brief APIのホストネーム。HTTPS化が非常にだるかった。
     *
     */
    static constexpr auto HOST_NAME = "share-everything-api.cosocaf.com";
    // static constexpr auto HOST_NAME = "localhost";
    /**
     * @brief APIのポート。
     * httpは80番ポート、httpsは443番ポート。
     * ローカルは8080番ポートを使用している。
     *
     */
    static constexpr auto PORT = "443";
    // static constexpr auto PORT = "8080";

    enum struct Method {
      Get,
      Post,
      Put,
      Delete,
    };

  public:
    ApiClient() noexcept;
    ~ApiClient() noexcept;

    /**
     * @brief ルームの値を持ってくる。
     *
     * @return Result<std::string, std::string>
     * 成功したらルームの値、失敗したらその原因を返す。
     */
    Result<std::string, std::string> getContent(std::string_view roomId);
    /**
     * @brief ルームの値を更新する。
     *
     * @param content 更新する値。
     * @return Result<_, std::string>
     * 成功したら何も返さず、失敗したらその原因を返す。
     */
    Result<_, std::string> putContent(std::string_view roomId,
                                      std::string_view content);

    /**
     * @brief ルームを新規作成する。
     *
     * @return Result<std::string, std::string>
     * 成功したらそのID、失敗したらその原因を返す。
     */
    Result<std::string, std::string> createRoom();
    /**
     * @brief ルームを削除する。
     * 削除したらIDは無効になるはず。
     *
     * @return Result<_, std::string>
     * 成功したら何も返さず、失敗したらその原因を返す。
     */
    Result<_, std::string> deleteRoom(std::string_view roomId);

  private:
    /**
     * @brief APIと通信する。通信結果の検証はApiClient::checkResponseで行う。
     *
     * @param to 送信先のURL
     * @param method 送信に使用するHTTPメソッド
     * @param body 送信に使用するHTTPボディ
     * @param contentType bodyのMIME
     * @return Result<std::string, std::string>
     * 成功したらレスポンスのボディ、失敗したらその原因。
     */
    Result<std::string, std::string> fetch(
      std::string_view to,
      Method method,
      std::optional<std::string_view> body = std::nullopt,
      std::optional<std::string_view> contentType
      = "application/json; charset=UTF-8");
    /**
     * @brief fetchの結果がAPIとして正しいものであるかを検証する。
     *
     * @param response fetchのレスポンス。
     * @return Result<rapidjson::Document, std::string>
     * 正しければJSON、そうでなければその原因。
     */
    Result<rapidjson::Document, std::string> checkResponse(
      std::string_view response);

    /**
     * @brief curlの通信で使用するコールバック。
     *
     * @see CURLのドキュメント
     *
     * @param buffer
     * @param size
     * @param nmemb
     * @param chunk
     * @return size_t
     */
    static size_t writeCallback(char* buffer,
                                size_t size,
                                size_t nmemb,
                                std::string* chunk);
    /**
     * @brief CURLのPUTで使用するコールバック。
     *
     * @see CURLのドキュメント
     *
     * @param buffer
     * @param size
     * @param nmemb
     * @param stream
     * @return size_t
     */
    static size_t readCallback(char* buffer,
                               size_t size,
                               size_t nmemb,
                               char* stream);
  };
} // namespace share_everything

#endif // SHARE_EVERYTHING_API_H_