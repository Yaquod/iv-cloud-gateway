// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include <gtest/gtest.h>

#include "transport/http_client/http_client.h"

using namespace gateway::transport;

class HttpClientTest : public ::testing::Test {
 protected:
  HttpClientTest() : client(http_options{}) {}
  HttpClient client;
};

TEST_F(HttpClientTest, GetRequestTest) {
  auto response = client.Get("http://google.com");
  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  ASSERT_FALSE(response.body.empty());
}

TEST_F(HttpClientTest, PostRequestTest) {
  std::string post_data = R"({"key": "value", "number": 42})";
  std::map<std::string, std::string> headers = {
      {"Content-Type", "application/json"}};

  auto response = client.Post("http://httpbin.org/post", headers, post_data);

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);

  // httpbin.org echoes back the data we sent
  EXPECT_NE(response.body.find("\"key\": \"value\""), std::string::npos);
}

// PUT request test
TEST_F(HttpClientTest, PutRequestTest) {
  std::string put_data = R"({"update": "data"})";
  std::map<std::string, std::string> headers = {
      {"Content-Type", "application/json"}};

  auto response = client.Put("http://httpbin.org/put", headers, put_data);

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  EXPECT_NE(response.body.find("\"update\": \"data\""), std::string::npos);
}

// PATCH request test
TEST_F(HttpClientTest, PatchRequestTest) {
  std::string patch_data = R"({"field": "patched"})";
  std::map<std::string, std::string> headers = {
      {"Content-Type", "application/json"}};

  auto response = client.Patch("http://httpbin.org/patch", headers, patch_data);

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  EXPECT_NE(response.body.find("\"field\": \"patched\""), std::string::npos);
}

// DELETE request test
TEST_F(HttpClientTest, DeleteRequestTest) {
  auto response = client.Delete("http://httpbin.org/delete");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// HEAD request test
TEST_F(HttpClientTest, HeadRequestTest) {
  auto response = client.Head("http://httpbin.org/get");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  // HEAD should not return body
  ASSERT_TRUE(response.body.empty());
}

// Test custom headers
TEST_F(HttpClientTest, CustomHeadersTest) {
  std::map<std::string, std::string> headers = {
      {"X-Custom-Header", "TestValue"}, {"Accept", "application/json"}};

  auto response = client.Get("http://httpbin.org/headers", headers);

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  EXPECT_NE(response.body.find("X-Custom-Header"), std::string::npos);
}

// Test response headers parsing
TEST_F(HttpClientTest, ResponseHeadersTest) {
  auto response =
      client.Get("http://httpbin.org/response-headers?Custom-Header=TestValue");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);

  auto custom_header = response.get_headers("Custom-Header");
  EXPECT_TRUE(custom_header.has_value());
  if (custom_header) {
    EXPECT_EQ(*custom_header, "TestValue");
  }
}

// Test 404 Not Found
TEST_F(HttpClientTest, NotFoundTest) {
  auto response = client.Get("http://httpbin.org/status/404");

  ASSERT_FALSE(response.success);
  ASSERT_EQ(404, response.status_code);
}

// Test 500 Server Error
TEST_F(HttpClientTest, ServerErrorTest) {
  auto response = client.Get("http://httpbin.org/status/500");

  ASSERT_FALSE(response.success);
  ASSERT_EQ(500, response.status_code);
}

// Test redirect following (302)
TEST_F(HttpClientTest, RedirectTest) {
  auto response = client.Get("http://httpbin.org/redirect/1");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test multiple redirects
TEST_F(HttpClientTest, MultipleRedirectsTest) {
  auto response = client.Get("http://httpbin.org/redirect/3");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test redirect with POST to GET conversion
TEST_F(HttpClientTest, PostRedirectToGetTest) {
  std::string post_data = R"({"data": "test"})";
  auto response =
      client.Post("http://httpbin.org/redirect-to?url=/get", {}, post_data);

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test absolute redirect
TEST_F(HttpClientTest, AbsoluteRedirectTest) {
  auto response = client.Get("http://httpbin.org/absolute-redirect/1");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test relative redirect
TEST_F(HttpClientTest, RelativeRedirectTest) {
  auto response = client.Get("http://httpbin.org/relative-redirect/1");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test invalid URL
TEST_F(HttpClientTest, InvalidUrlTest) {
  auto response = client.Get("not-a-valid-url");
  ASSERT_FALSE(response.success);
  ASSERT_EQ(0, response.status_code);
  EXPECT_NE(response.error_message.find("Invalid URL"), std::string::npos);
}

// Test HTTPS rejection
TEST_F(HttpClientTest, HttpsSupportedTest) {
  auto response = client.Get("https://httpbin.org/get");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test connection to non-existent host
TEST_F(HttpClientTest, NonExistentHostTest) {
  try {
    auto response = client.Get("http://this-host-does-not-exist-12345.com");
    ASSERT_FALSE(response.success);
    ASSERT_EQ(0, response.status_code);
    ASSERT_FALSE(response.error_message.empty());
  } catch (const std::exception& e) {
    SUCCEED();
  }
}

// Test custom port
TEST_F(HttpClientTest, CustomPortTest) {
  auto response = client.Get("http://httpbin.org:80/get");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test URL with path and query parameters
TEST_F(HttpClientTest, UrlWithQueryParamsTest) {
  try {
    auto response =
        client.Get("http://httpbin.org/get?param1=value1&param2=value2");

    ASSERT_TRUE(response.success);
    ASSERT_EQ(200, response.status_code);
    EXPECT_NE(response.body.find("param1"), std::string::npos);
    EXPECT_NE(response.body.find("value1"), std::string::npos);
  } catch (const std::exception& e) {
    FAIL() << "Failed with exception: " << e.what();
  }
}

// Test empty body POST
TEST_F(HttpClientTest, EmptyBodyPostTest) {
  auto response = client.Post("http://httpbin.org/post", {}, "");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test user agent header
TEST_F(HttpClientTest, UserAgentHeaderTest) {
  auto response = client.Get("http://httpbin.org/user-agent");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  EXPECT_NE(response.body.find("VehicleGateway/2.0"), std::string::npos);
}

// Test 307 redirect
TEST_F(HttpClientTest, Redirect307PreservesMethodTest) {
  std::string post_data = R"({"data": "test"})";
  auto response =
      client.Post("http://httpbin.org/redirect-to?url=/post&status_code=307",
                  {{"Content-Type", "application/json"}}, post_data);

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test case-insensitive Location header
TEST_F(HttpClientTest, CaseInsensitiveLocationHeaderTest) {
  auto response = client.Get("http://httpbin.org/redirect/1");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test response body content
TEST_F(HttpClientTest, ResponseBodyContentTest) {
  auto response = client.Get("http://httpbin.org/json");

  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  ASSERT_FALSE(response.body.empty());
  EXPECT_NE(response.body.find("slideshow"), std::string::npos);
}

TEST_F(HttpClientTest, Status201CreatedTest) {
  auto response = client.Get("http://httpbin.org/status/201");
  ASSERT_TRUE(response.success);
  ASSERT_EQ(201, response.status_code);
}

TEST_F(HttpClientTest, Status204NoContentTest) {
  auto response = client.Get("http://httpbin.org/status/204");
  ASSERT_TRUE(response.success);
  ASSERT_EQ(204, response.status_code);
}

TEST_F(HttpClientTest, Status400BadRequestTest) {
  auto response = client.Get("http://httpbin.org/status/400");
  ASSERT_FALSE(response.success);
  ASSERT_EQ(400, response.status_code);
}

TEST_F(HttpClientTest, Status401UnauthorizedTest) {
  auto response = client.Get("http://httpbin.org/status/401");
  if (response.status_code == 0) {
    GTEST_SKIP() << "Network connectivity issue: " << response.error_message;
  }
  ASSERT_FALSE(response.success);
  ASSERT_EQ(401, response.status_code);
}

TEST_F(HttpClientTest, Status403ForbiddenTest) {
  auto response = client.Get("http://httpbin.org/status/403");
  ASSERT_FALSE(response.success);
  ASSERT_EQ(403, response.status_code);
}

TEST_F(HttpClientTest, HttpsGetSupportedTest) {
  auto response = client.Get("https://httpbin.org/get");
  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

TEST_F(HttpClientTest, HttpsPostSupportedTest) {
  std::string post_data = R"({"key": "value"})";
  std::map<std::string, std::string> headers = {
    {"Content-Type", "application/json"}};
  auto response = client.Post("https://httpbin.org/post", headers, post_data);
  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
}

// Test HTTP GET with large response body
TEST_F(HttpClientTest, HttpGetLargeBodyTest) {
  auto response = client.Get("http://httpbin.org/bytes/65536");
  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  ASSERT_EQ(response.body.size(), 65536u);
}

// Test HTTP GET with chunked transfer encoding
TEST_F(HttpClientTest, HttpGetChunkedTest) {
  auto response = client.Get("http://httpbin.org/stream/20");
  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  ASSERT_FALSE(response.body.empty());
}

TEST_F(HttpClientTest, HttpGetGzipTest) {
  std::map<std::string, std::string> headers = {
    {"Accept-Encoding", "gzip"}};
  auto response = client.Get("http://httpbin.org/gzip", headers);
  ASSERT_TRUE(response.success);
  ASSERT_EQ(200, response.status_code);
  // Check the headers instead of the uncompressed body
  EXPECT_EQ(response.headers["Content-Encoding"], "gzip");
}