// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include "http_client.h"

#include <spdlog/spdlog.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
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
    std::regex re(R"(^(http)://([^:/]+)(?::(\d+))?(/.*)?$)");
    std::smatch match;
    if (!std::regex_match(url, match, re)) {
      throw std::runtime_error("Invalid URL: " + url);
    }
    Url uri;
    uri.protocol = match[1];
    uri.host = match[2];
    uri.port = match[3].length() > 0 ? match[3].str()
                                     : (uri.protocol == "https" ? "443" : "80");
    uri.path = match[4].length() > 0 ? match[4].str() : "/";
    return uri;
  }

  http_response do_request(const Url& url, http::verb method,
                           const std::string& body,
                           const std::map<std::string, std::string>& headers) {
    http_response response;
    try {
      tcp::resolver resolver(ioc);
      auto const results = resolver.resolve(url.host, url.port);

      beast::tcp_stream stream(ioc);
      stream.connect(results);
      stream.expires_after(options.timeout);

      // Build request
      http::request<http::string_body> req{method, url.path, 11};
      req.set(http::field::host, url.host);
      req.set(http::field::user_agent, "VehicleGateway/1.0");

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
      // Send request
      http::write(stream, req);

      // Receive the response
      beast::flat_buffer buffer;
      http::response<http::string_body> res;
      http::read(stream, buffer, res);

      response.status_code = static_cast<int>(res.result_int());
      response.body = res.body();

      for (const auto& field : res) {
        response.headers[std::string(field.name_string())] =
            std::string(field.value());
      }
      response.success =
          (response.status_code >= 200 && response.status_code < 300);

      // Gracefully close the socket
      beast::error_code ec;
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);
      if (ec && ec != boost::asio::error::not_connected) {
        spdlog::error("Error while closing socket: {}", ec.message());
      }
    } catch (const std::exception& e) {
      spdlog::error("Error while sending request: {}", e.what());
      response.success = false;
      response.status_code = 0;
      response.error_message = e.what();
    }
    return response;
  }

  std::string resolve_redirect(const std::string& url,
                               const std::string& location) {
    if (location.rfind("http", 0) == 0 || location.rfind("https", 0) == 0) {
      return location;
    }
    auto uri = parse_url(url);
    if (!location.empty() && location[0] == '/') {
      return uri.protocol + "://" + uri.host + ":" +
             (uri.port != "80" && uri.port != "443" ? ":" + uri.port : "") +
             location;
    }

    size_t last_slash = uri.path.find_last_of('/');
    if (last_slash != std::string::npos) {
      uri.path.resize(last_slash);
    }

    return uri.protocol + "://" + uri.host + ":" +
           (uri.port != "80" && uri.port != "443" ? ":" + uri.port : "") +
           uri.path + location;
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
      std::string redirect_method =
          (response.status_code == 307 || response.status_code == 308) ? "GET"
                                                                       : method;
      std::string redirect_body = (redirect_method == "GET") ? "" : body;
      return do_request(redirect_method, new_url, headers, redirect_body,
                        redirects_remaining - 1);
    } else {
      spdlog::warn("Redirects response without header", response.status_code);
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
