/*
 * Copyright 2026 wafdy
 * Copyright 2026 Alaa Hassan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IV_CLOUD_GATEWAY_HTTP_CLIENT_H
#define IV_CLOUD_GATEWAY_HTTP_CLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace gateway::transport {

struct http_response {
  int status_code;
  std::string body;
  std::map<std::string, std::string> headers;
  bool success;
  std::string error_message;

  [[nodiscard]] bool is_redirect() const {
    return status_code == 301 || status_code == 302 || status_code == 307 ||
           status_code == 308;
  }

  [[nodiscard]] std::optional<std::string> get_headers(
      const std::string& name) const {
    auto it = headers.find(name);
    if (it != headers.end()) return it->second;
    // case-insensitive fallback for "location" / "Location"
    for (auto& [k, v] : headers)
      if (k == name) return v;
    return std::nullopt;
  }
};

struct http_options {
  std::chrono::seconds timeout{30};
  int max_redirects{5};
  bool follow_redirects{true};
  bool restrict_redirect_to_same_host{false};
};

class HttpClient {
 public:
  explicit HttpClient(const http_options& options);
  ~HttpClient();

  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;

  HttpClient(HttpClient&&) noexcept;
  HttpClient& operator=(HttpClient&&) noexcept;

  [[nodiscard]] http_response Get(
      const std::string& url,
      const std::map<std::string, std::string>& headers = {}) const;

  [[nodiscard]] http_response Post(
      const std::string& url,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "") const;

  [[nodiscard]] http_response Put(
      const std::string& url,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "") const;

  [[nodiscard]] http_response Patch(
      const std::string& url,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "") const;

  [[nodiscard]] http_response Delete(
      const std::string& url,
      const std::map<std::string, std::string>& headers = {}) const;

  [[nodiscard]] http_response Head(
      const std::string& url,
      const std::map<std::string, std::string>& headers = {}) const;

 private:
  struct impl;
  std::unique_ptr<impl> pimpl_;

  [[nodiscard]] http_response do_request(
      const std::string& method, const std::string& url,
      const std::map<std::string, std::string>& headers,
      const std::string& body, int redirects_remaining) const;
};

}  // namespace gateway::transport
#endif  // IV_CLOUD_GATEWAY_HTTP_CLIENT_H
