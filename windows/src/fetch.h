#ifndef SHARE_EVERYTHING_FETCH_H_
#define SHARE_EVERYTHING_FETCH_H_

#include "pch.h"

#include <map>
#include <optional>

#include "tchar_util.h"

namespace share_everything {
  enum struct HttpMethod {
    Get,
    Post,
    Put,
    Delete,
  };
  struct HttpRequest {
    tstring httpVersion;
    tstring hostName;
    tstring port;
    HttpMethod method;
    tstring path;
    std::map<tstring, tstring> headers;
    tstring body;
    HttpRequest(HttpMethod method,
                tstring_view path,
                const std::map<tstring, tstring>& headers,
                tstring_view body = _T(""));
  };
  struct HttpResponse {
    tstring httpVersion;
    int status;
    tstring statusMessage;
    std::map<tstring, tstring> headers;
    tstring body;
  };
  std::optional<HttpResponse> fetch(const HttpRequest& request);
} // namespace share_everything

#endif // SHARE_EVERYTHING_FETCH_H_