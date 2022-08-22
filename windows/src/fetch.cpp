#include "fetch.h"

#include <cassert>
#include <format>
#include <thread>
#include <vector>

#include "logger.h"

namespace share_everything {
  namespace {
    struct WinSock {
      WSADATA wsaData;
      LPADDRINFO addrInfo;
      SOCKET connectSocket;

      WinSock()
        : wsaData({}), addrInfo(nullptr), connectSocket(INVALID_SOCKET) {}
      ~WinSock() {
        if (connectSocket != INVALID_SOCKET) {
          closesocket(connectSocket);
          connectSocket = INVALID_SOCKET;
        }
        if (addrInfo != nullptr) {
          freeaddrinfo(addrInfo);
          addrInfo = nullptr;
        }
        WSACleanup();
      }
      BOOL init() {
        LOG_INFO(_T("Initializing WinSock."), _T("Fetch"));

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
          LOG_ERROR(_T("Failed to WSA startup."), _T("Fetch"));
          return false;
        }

        LOG_INFO(_T("WinSock has been initialized."), _T("Fetch"));
        return true;
      }

      BOOL connect(const HttpRequest& request) {
        ADDRINFOT hints{};
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        ADDRINFOT* addrinfo;

        if (GetAddrInfo(
              request.hostName.c_str(), request.port.c_str(), &hints, &addrinfo)
            != 0) {
          LOG_ERROR(_T("Failed to resolve address."), _T("Fetch"));
          return false;
        }

        for (auto ptr = addrinfo; ptr != nullptr; ptr = ptr->ai_next) {
          connectSocket = socket(
            addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
          if (connectSocket == INVALID_SOCKET) {
            LOG_ERROR(_T("Failed to connect socket."), _T("Fetch"));
            return false;
          }

          auto connectResult = ::connect(
            connectSocket, addrinfo->ai_addr, (int)addrinfo->ai_addrlen);
          if (connectResult != SOCKET_ERROR) {
            break;
          }

          closesocket(connectSocket);
          connectSocket = INVALID_SOCKET;
        }

        if (connectSocket == INVALID_SOCKET) {
          LOG_ERROR(_T("Failed to connect socket."), _T("Fetch"));
          return false;
        }

        LOG_INFO(_T("Socket successfully connected."), _T("Fetch"));

        return true;
      }
    };

    BOOL sendMessage(SOCKET socket, std::string_view message) {
      LOG_INFO(std::format(_T("Send message:\n{}"), strToTstr(message)),
               _T("Fetch"));
      assert(socket != INVALID_SOCKET);

      size_t bufSize  = message.size();
      const char* buf = message.data();

      size_t sentSize = 0;
      while (sentSize < bufSize) {
        auto size = send(
          socket, buf + sentSize, static_cast<int>(bufSize - sentSize), 0);
        if (size == SOCKET_ERROR) {
          LOG_ERROR(
            std::format(_T("Failed to send message: {}"), strToTstr(message)),
            _T("Fetch"));
          break;
        }
        sentSize += size;
      }

      if (sentSize < bufSize) {
        return false;
      }
      return true;
    }

    std::optional<HttpResponse> decodeResponse(tstring_view data) {
      LOG_INFO(_T("Start decoding response."), _T("Fetch"));

      std::vector<tstring_view> headerPart;
      while (!data.empty()) {
        auto pos = data.find(_T("\r\n"));
        if (pos == data.npos) {
          LOG_ERROR(
            _T("Invalid response. The header line does not end with \\r\\n."),
            _T("Fetch"));
          return std::nullopt;
        }
        auto line = data.substr(0, pos);
        data      = data.substr(pos + 2);
        if (line.empty()) {
          break;
        }
        headerPart.push_back(line);
      }

      if (headerPart.empty()) {
        LOG_ERROR(_T("Invalid response. The header part was empty."),
                  _T("Fetch"));
        return std::nullopt;
      }

      HttpResponse response;

      auto statusLine      = headerPart[0];
      auto pos             = statusLine.find(_T(" "));
      response.httpVersion = statusLine.substr(0, pos);

      statusLine      = statusLine.substr(pos + 1);
      pos             = statusLine.find(_T(" "));
      auto statusStr  = statusLine.substr(0, pos);
      response.status = std::stoi(tstring(statusStr));

      response.statusMessage = statusLine.substr(pos + 1);

      for (size_t i = 1; i < headerPart.size(); ++i) {
        pos = headerPart[i].find(_T(": "));
        tstring key(headerPart[i].substr(0, pos));
        response.headers[key] = headerPart[i].substr(pos + 2);
      }

      response.body = data;

      LOG_INFO(_T("Response successfully decoded."), _T("Fetch"));

      return response;
    }
  } // namespace

  HttpRequest::HttpRequest(HttpMethod method,
                           tstring_view path,
                           const std::map<tstring, tstring>& headers,
                           tstring_view body)
    : httpVersion(_T("HTTP/1.1")),
      hostName(_T("localhost")),
      port(_T("8080")),
      method(method),
      path(path),
      headers(headers),
      body(body) {}

  std::optional<HttpResponse> fetch(const HttpRequest& request) {
    LOG_INFO(_T("Start fetch."), _T("Fetch"));

    WinSock winSock;
    if (!winSock.init()) {
      return std::nullopt;
    }
    if (!winSock.connect(request)) {
      return std::nullopt;
    }
    auto socket = winSock.connectSocket;

    std::string method;
    switch (request.method) {
      case HttpMethod::Get:
        method = "GET";
        break;
      case HttpMethod::Post:
        method = "POST";
        break;
      case HttpMethod::Put:
        method = "PUT";
        break;
      case HttpMethod::Delete:
        method = "DELETE";
        break;
      default:
        LOG_ERROR(
          _T("Invalid method. The only permitted methods are Get, Post, Put, and Delete."),
          _T("Fetch"));
        return std::nullopt;
    }

    std::string body = tstrToStr(request.body);

    std::map<std::string, std::string> headers;
    for (const auto& [key, value] : request.headers) {
      headers[tstrToStr(key)] = tstrToStr(value);
    }
    if (headers.find("Host") == headers.end()) {
      headers["Host"] = tstrToStr(request.hostName);
    }
    if (headers.find("Content-Length") == headers.end()) {
      headers["Content-Length"] = std::to_string(body.size());
    }

    std::string message;
    message += std::format("{} {} {}\r\n",
                           method,
                           tstrToStr(request.path),
                           tstrToStr(request.httpVersion));

    for (const auto& [key, value] : headers) {
      message += std::format("{}: {}\r\n", key, value);
    }

    message += "\r\n";

    message += body;

    if (!sendMessage(socket, message)) {
      LOG_ERROR(_T("Failed to fetch."), _T("Fetch"));
      return std::nullopt;
    }

    tstring data;
    while (true) {
      char buf[512];
      auto result = recv(socket, buf, sizeof(buf), 0);
      if (result > 0) {
        if (shutdown(socket, SD_SEND) == SOCKET_ERROR) {
          return std::nullopt;
        }
#ifdef _UNICODE
        int wbufSize = MultiByteToWideChar(CP_UTF8, 0, buf, result, nullptr, 0);
        auto wbuf    = new wchar_t[wbufSize + 1];
        MultiByteToWideChar(CP_UTF8, 0, buf, result, wbuf, wbufSize);
        wbuf[wbufSize] = L'\0';
        data += wbuf;
        delete[] wbuf;
#else
        data += buf;
#endif // _UNICODE
      } else if (result == 0) {
        break;
      } else {
        LOG_ERROR(_T("Failed to recv."), _T("Fetch"));
        return std::nullopt;
      }
    }

    LOG_INFO(_T("Successfully requested.\n") + data, _T("Fetch"));

    auto response = decodeResponse(data);
    if (response) {
      LOG_INFO(_T("Successfully fetched."), _T("Fetch"));
    }

    return response;
  }
} // namespace share_everything