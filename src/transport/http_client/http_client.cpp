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

#include "http_client.h"

#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include <spdlog/spdlog.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <regex>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

namespace gateway::transport {

struct Url {
  std::string protocol;
  std::string host;
  std::string port;
  std::string path;
};

static std::optional<Url> parse_url(const std::string& url) {
  // protocol://host:port/path?query#fragment
  static const std::regex re(
      R"(^(https?)://([^:/\?#]+)(?::(\d+))?([^\?#]*)?(?:\?([^#]*))?(?:#(.*))?$)");
  std::smatch m;
  if (!std::regex_match(url, m, re)) {
    spdlog::error("[HTTP] invalid URL: {}", url);
    return std::nullopt;
  }
  Url uri;
  uri.protocol = m[1];
  uri.host = m[2];
  uri.port =
      m[3].length() > 0 ? m[3].str() : (uri.protocol == "https" ? "443" : "80");

  // Build the full path including query parameters
  std::string path = m[4].length() > 0 ? m[4].str() : "/";
  std::string query = m[5].str();

  uri.path = query.empty() ? path : path + "?" + query;

  return uri;
}

static std::string resolve_redirect(const std::string& base,
                                    const std::string& location) {
  if (location.rfind("http://", 0) == 0 || location.rfind("https://", 0) == 0)
    return location;
  auto u = parse_url(base);
  std::string root = u->protocol + "://" + u->host;
  if ((u->protocol == "http" && u->port != "80") ||
      (u->protocol == "https" && u->port != "443"))
    root += ":" + u->port;
  if (!location.empty() && location[0] == '/') return root + location;
  size_t sl = u->path.find_last_of('/');
  std::string dir = (sl != std::string::npos) ? u->path.substr(0, sl + 1) : "/";
  return root + dir + location;
}

struct HttpClient::impl {
  net::io_context ioc;
  http_options options;

  http_response do_request(const Url& url, http::verb verb,
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

      if (url.protocol == "https") {
        ssl::context ctx{ssl::context::tls_client};
        ctx.set_default_verify_paths();
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        if (!stream.native_handle()) {
          spdlog::error("Fatal OpenSSL Error: SSL native handle is null.");
          response.success = false;
          response.status_code = 0;
          response.error_message = "Failed to initialize OpenSSL stream";
          return response;
        }

        if (!SSL_set_tlsext_host_name(stream.native_handle(),
                                      url.host.c_str())) {
          ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                                 net::error::get_ssl_category());
          spdlog::error("Error setting SNI hostname: {}", ec.message());
          response.success = false;
          response.status_code = 0;
          response.error_message = "SNI setup failed: " + ec.message();
          return response;
        }

        beast::get_lowest_layer(stream).expires_after(options.timeout);
        beast::get_lowest_layer(stream).connect(results, ec);
        if (ec) {
          spdlog::error("Error connecting to {}:{}: {}", url.host, url.port,
                        ec.message());
          response.success = false;
          response.status_code = 0;
          response.error_message = "Connecting failed: " + ec.message();
          return response;
        }

        stream.handshake(ssl::stream_base::client, ec);
        if (ec) {
          spdlog::error("SSL handshake failed with {}:{}: {}", url.host,
                        url.port, ec.message());
          response.success = false;
          response.status_code = 0;
          response.error_message = "SSL handshake failed: " + ec.message();
          return response;
        }

        return send_and_receive(stream, url, verb, body, headers, ec);
      } else {
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
        return send_and_receive(stream, url, verb, body, headers, ec);
      }
    } catch (const std::exception& e) {
      spdlog::error("Unexpected error during request: {}", e.what());
      response.success = false;
      response.status_code = 0;
      response.error_message = std::string("Exception: ") + e.what();
    }

    return response;
  }

  template <typename Stream>
  http_response send_and_receive(
      Stream& stream, const Url& url, http::verb verb, const std::string& body,
      const std::map<std::string, std::string>& headers,
      beast::error_code& ec) {
    http_response resp{};

    http::request<http::string_body> req{verb, url.path, 11};
    req.set(http::field::host, url.host);
    req.set(http::field::user_agent, "VehicleGateway/2.0");
    req.set(http::field::connection, "close");
    for (auto& [k, v] : headers) req.set(k, v);
    if (!body.empty()) {
      req.body() = body;
      req.prepare_payload();
    }

    get_lowest_layer(stream).expires_after(options.timeout);
    http::write(stream, req, ec);
    if (ec) {
      resp.error_message = "write failed: " + ec.message();
      return resp;
    }

    beast::flat_buffer buf;
    http::response_parser<http::string_body> parser;
    parser.body_limit(std::numeric_limits<uint64_t>::max());

    if (verb == http::verb::head) {
      parser.skip(true);
    }
    get_lowest_layer(stream).expires_after(options.timeout);
    http::read(stream, buf, parser, ec);

    if (ec && ec != http::error::end_of_stream &&
        ec != boost::asio::error::eof) {
      if (ec == http::error::partial_message && parser.is_done()) {
        // acceptable
      } else {
        resp.error_message = "read failed: " + ec.message();
        return resp;
      }
    }

    auto res = parser.release();
    resp.status_code = static_cast<int>(res.result_int());
    resp.body = res.body();
    for (auto& f : res)
      resp.headers[std::string(f.name_string())] = std::string(f.value());
    resp.success = resp.status_code >= 200 && resp.status_code < 300;

    return resp;
  }
};

HttpClient::HttpClient(const http_options& options)
    : pimpl_(std::make_unique<impl>()) {
  pimpl_->options = options;
}

HttpClient::HttpClient(HttpClient&&) noexcept = default;

HttpClient& HttpClient::operator=(HttpClient&&) noexcept = default;

HttpClient::~HttpClient() = default;

http_response HttpClient::do_request(
    const std::string& method, const std::string& url,
    const std::map<std::string, std::string>& headers, const std::string& body,
    int redirects_remaining) const {
  auto uri = parse_url(url);
  if (!uri) {
    return http_response{0, "", {}, false, "Invalid URL: " + url};
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

  auto response = pimpl_->do_request(uri.value(), verb, body, headers);

  if (pimpl_->options.follow_redirects && response.is_redirect() &&
      redirects_remaining > 0) {
    // Use get_headers for case-insensitive lookup
    auto location = response.get_headers("Location");
    if (!location) {
      location = response.get_headers("location");
    }
    if (location) {
      auto new_url = resolve_redirect(url, *location);
      spdlog::info("[HTTP] redirect → {}", new_url);
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

http_response HttpClient::Get(
    const std::string& url,
    const std::map<std::string, std::string>& headers) const {
  return do_request("GET", url, headers, "", pimpl_->options.max_redirects);
}

http_response HttpClient::Post(
    const std::string& url, const std::map<std::string, std::string>& headers,
    const std::string& body) const {
  return do_request("POST", url, headers, body, pimpl_->options.max_redirects);
}

http_response HttpClient::Put(const std::string& url,
                              const std::map<std::string, std::string>& headers,
                              const std::string& body) const {
  return do_request("PUT", url, headers, body, pimpl_->options.max_redirects);
}

http_response HttpClient::Patch(
    const std::string& url, const std::map<std::string, std::string>& headers,
    const std::string& body) const {
  return do_request("PATCH", url, headers, body, pimpl_->options.max_redirects);
}

http_response HttpClient::Delete(
    const std::string& url,
    const std::map<std::string, std::string>& headers) const {
  return do_request("DELETE", url, headers, "", pimpl_->options.max_redirects);
}

http_response HttpClient::Head(
    const std::string& url,
    const std::map<std::string, std::string>& headers) const {
  return do_request("HEAD", url, headers, "", pimpl_->options.max_redirects);
}
}  // namespace gateway::transport
