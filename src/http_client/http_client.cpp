// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include "http_client.h"

#include <spdlog/spdlog.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <regex>

namespace cloud_gateway {

struct HttpClient::impl {
  net::io_context ioc;
  http_options options;

  struct Url {
    std::string protocol;
    std::string host;
    std::string port;
    std::string path;
  };

  Url parse_url(const std::string& url) {
    // protocol://host:port/path?query#fragment
    std::regex re(
        R"(^(https?)://([^:/\?#]+)(?::(\d+))?([^\?#]*)?(?:\?([^#]*))?(?:#(.*))?$)");
    std::smatch match;
    if (!std::regex_match(url, match, re)) {
      throw std::runtime_error("Invalid URL: " + url);
    }
    Url uri;
    uri.protocol = match[1];
    uri.host = match[2];
    uri.port = match[3].length() > 0 ? match[3].str()
                                     : (uri.protocol == "https" ? "443" : "80");

    // Build the full path including query parameters
    std::string path = match[4].length() > 0 ? match[4].str() : "/";
    std::string query = match[5].str();

    if (!query.empty()) {
      uri.path = path + "?" + query;
    } else {
      uri.path = path.empty() ? "/" : path;
    }

    return uri;
  }

  http_response do_request(const Url& url, http::verb method,
                           const std::string& body,
                           const std::map<std::string, std::string>& headers) {
    http_response response;
    beast::error_code ec;

    try {
      tcp::resolver resolver(ioc);
      auto const results = resolver.resolve(url.host, url.port, ec);

      if (ec) {
        spdlog::error("Error resolving host {}: {}", url.host, ec.message());
        response.success = false;
        response.status_code = 0;
        response.error_message = "DNS resolution failed: " + ec.message();
        return response;
      }

      beast::tcp_stream stream(ioc);

      // Set timeout for connection
      stream.expires_after(options.timeout);

      stream.connect(results, ec);

      if (ec) {
        spdlog::error("Error connecting to {}:{}: {}", url.host, url.port,
                      ec.message());
        response.success = false;
        response.status_code = 0;
        response.error_message = "Connection failed: " + ec.message();
        return response;
      }

      // Build request
      http::request<http::string_body> req{method, url.path, 11};
      req.set(http::field::host, url.host);
      req.set(http::field::user_agent, "VehicleGateway/1.0");
      req.set(http::field::connection, "close");

      for (const auto& [key, value] : options.default_headers) {
        req.set(key, value);
      }

      for (const auto& [key, value] : headers) {
        req.set(key, value);
      }

      // Check for POST/PUT/PATCH
      if (!body.empty()) {
        req.body() = body;
        req.prepare_payload();
      }

      // Set timeout for write operation
      stream.expires_after(options.timeout);

      // Send request
      http::write(stream, req, ec);

      if (ec) {
        spdlog::error("Error writing request: {}", ec.message());
        response.success = false;
        response.status_code = 0;
        response.error_message = "Write failed: " + ec.message();
        return response;
      }

      // Set timeout for read operation
      stream.expires_after(options.timeout);

      // Use response_parser for more robust parsing
      beast::flat_buffer buffer;
      http::response_parser<http::string_body> parser;

      // Allow for partial messages when connection closes
      parser.body_limit(std::numeric_limits<std::uint64_t>::max());

      // For HEAD requests, skip the body
      if (method == http::verb::head) {
        parser.skip(true);
      }

      // Read the response
      http::read(stream, buffer, parser, ec);

      if (ec == http::error::end_of_stream || ec == boost::asio::error::eof) {
        spdlog::debug(
            "Connection closed by server (normal for Connection: close)");
      } else if (ec == http::error::partial_message) {
        spdlog::warn("Received partial message from server");
        // For HEAD requests or if parser is done, this is acceptable
        if (method != http::verb::head && !parser.is_done()) {
          spdlog::error("Incomplete response received");
          response.success = false;
          response.status_code = 0;
          response.error_message = "Incomplete response: " + ec.message();
          return response;
        } else {
          spdlog::debug(
              "Partial message acceptable for HEAD or complete response");
        }
      } else if (ec) {
        spdlog::error("Error reading response: {}", ec.message());
        response.success = false;
        response.status_code = 0;
        response.error_message = "Read failed: " + ec.message();
        return response;
      }

      // Extract the response
      auto res = parser.release();

      response.status_code = static_cast<int>(res.result_int());
      response.body = res.body();

      for (const auto& field : res) {
        response.headers[std::string(field.name_string())] =
            std::string(field.value());
      }
      response.success =
          (response.status_code >= 200 && response.status_code < 300);

      // Gracefully close the socket
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);
      if (ec && ec != boost::asio::error::not_connected &&
          ec != boost::asio::error::eof) {
        spdlog::warn("Error while closing socket: {}", ec.message());
      }

    } catch (const std::exception& e) {
      spdlog::error("Unexpected error during request: {}", e.what());
      response.success = false;
      response.status_code = 0;
      response.error_message = std::string("Exception: ") + e.what();
    }

    return response;
  }

  std::string resolve_redirect(const std::string& url,
                               const std::string& location) {
    if (location.rfind("http://", 0) == 0 ||
        location.rfind("https://", 0) == 0) {
      return location;
    }

    auto uri = parse_url(url);

    // Build base URL with optional port
    std::string base_url = uri.protocol + "://" + uri.host;

    if ((uri.protocol == "http" && uri.port != "80") ||
        (uri.protocol == "https" && uri.port != "443")) {
      base_url += ":" + uri.port;
    }

    if (!location.empty() && location[0] == '/') {
      return base_url + location;
    }

    size_t last_slash = uri.path.find_last_of('/');
    std::string current_dir;

    if (last_slash != std::string::npos) {
      current_dir = uri.path.substr(0, last_slash + 1);
    } else {
      current_dir = "/";
    }

    return base_url + current_dir + location;
  }
};

HttpClient::HttpClient() : pimpl_(std::make_unique<impl>()) {}

HttpClient::HttpClient(const http_options& options)
    : pimpl_(std::make_unique<impl>()) {
  pimpl_->options = options;
}

HttpClient::HttpClient(HttpClient&&) noexcept = default;

HttpClient& HttpClient::operator=(HttpClient&&) noexcept = default;

HttpClient::~HttpClient() = default;

// cppcheck-suppress unusedFunction
void HttpClient::set_timeout(const std::chrono::seconds timeout) const {
  pimpl_->options.timeout = timeout;
}

// cppcheck-suppress unusedFunction
void HttpClient::set_max_redirects(int max_redirects) {
  pimpl_->options.max_redirects = max_redirects;
}

// cppcheck-suppress unusedFunction
void HttpClient::set_follow_redirects(bool follow_redirects) {
  pimpl_->options.follow_redirects = follow_redirects;
}

http_response HttpClient::do_request(
    const std::string& method, const std::string& url,
    const std::map<std::string, std::string>& headers, const std::string& body,
    int redirects_remaining) const {
  auto uri = pimpl_->parse_url(url);

  // check for protocol
  if (uri.protocol == "https") {
    spdlog::error("HTTPS not supported yet");
    return http_response{0, "", {}, false, "HTTPS not supported yet"};
  }

  http::verb verb = http::verb::get;
  if (method == "POST") {
    verb = http::verb::post;
  } else if (method == "PUT") {
    verb = http::verb::put;
  } else if (method == "PATCH") {
    verb = http::verb::patch;
  } else if (method == "DELETE") {
    verb = http::verb::delete_;
  } else if (method == "HEAD") {
    verb = http::verb::head;
  }

  auto response = pimpl_->do_request(uri, verb, body, headers);

  // handle redirects
  if (pimpl_->options.follow_redirects && response.is_redirect() &&
      redirects_remaining > 0) {
    auto location = response.get_headers("Location");
    if (!location) {
      location = response.get_headers("location");
    }
    if (location) {
      auto new_url = pimpl_->resolve_redirect(url, *location);
      spdlog::info("Redirecting to {}", new_url);

      std::string redirect_method;
      std::string redirect_body;

      // 307 and 308 preserve the original method and body
      if (response.status_code == 307 || response.status_code == 308) {
        redirect_method = method;
        redirect_body = body;
      }
      // 301, 302, 303 convert POST/PUT/PATCH to GET
      else if (method == "POST" || method == "PUT" || method == "PATCH") {
        redirect_method = "GET";
        redirect_body = "";
      }
      // GET, DELETE, HEAD remain unchanged
      else {
        redirect_method = method;
        redirect_body = "";
      }

      return do_request(redirect_method, new_url, headers, redirect_body,
                        redirects_remaining - 1);
    } else {
      spdlog::warn("Redirect response without Location header, status: {}",
                   response.status_code);
    }
  }
  return response;
}

// cppcheck-suppress unusedFunction
http_response HttpClient::Get(
    const std::string& url,
    const std::map<std::string, std::string>& headers) const {
  return do_request("GET", url, headers, "", pimpl_->options.max_redirects);
}

// cppcheck-suppress unusedFunction
http_response HttpClient::Post(
    const std::string& url, const std::map<std::string, std::string>& headers,
    const std::string& body) const {
  return do_request("POST", url, headers, body, pimpl_->options.max_redirects);
}

// cppcheck-suppress unusedFunction
http_response HttpClient::Put(const std::string& url,
                              const std::map<std::string, std::string>& headers,
                              const std::string& body) const {
  return do_request("PUT", url, headers, body, pimpl_->options.max_redirects);
}

// cppcheck-suppress unusedFunction
http_response HttpClient::Patch(
    const std::string& url, const std::map<std::string, std::string>& headers,
    const std::string& body) const {
  return do_request("PATCH", url, headers, body, pimpl_->options.max_redirects);
}

// cppcheck-suppress unusedFunction
http_response HttpClient::Delete(
    const std::string& url,
    const std::map<std::string, std::string>& headers) const {
  return do_request("DELETE", url, headers, "", pimpl_->options.max_redirects);
}

// cppcheck-suppress unusedFunction
http_response HttpClient::Head(
    const std::string& url,
    const std::map<std::string, std::string>& headers) const {
  return do_request("HEAD", url, headers, "", pimpl_->options.max_redirects);
}
}  // namespace cloud_gateway
