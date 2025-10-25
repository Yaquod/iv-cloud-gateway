// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#ifndef IV_CLOUD_GATEWAY_HTTP_CLIENT_H
#define IV_CLOUD_GATEWAY_HTTP_CLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <map>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace cloud_gateway {

struct http_response;
struct http_options;

class HttpClient {
 public:
  HttpClient();
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

  // TODO: add other methods for ssl and https.

  void set_timeout(std::chrono::seconds timeout) const;

  void set_max_redirects(int max_redirects);

  void set_follow_redirects(bool follow_redirects);

 private:
  struct impl;
  std::unique_ptr<impl> pimpl_;

  [[nodiscard]] http_response do_request(
      const std::string& method, const std::string& url,
      const std::map<std::string, std::string>& headers,
      const std::string& body, int redirects_remaining) const;
};

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

  std::optional<std::string> get_headers(const std::string& name) {
    auto it = headers.find(name);
    return it != headers.end() ? std::optional<std::string>(it->second)
                               : std::nullopt;
  }
};

struct http_options {
  std::chrono::seconds timeout{30};
  int max_redirects{5};
  bool follow_redirects{true};
  std::map<std::string, std::string> default_headers;
};

}  // namespace cloud_gateway
#endif  // IV_CLOUD_GATEWAY_HTTP_CLIENT_H