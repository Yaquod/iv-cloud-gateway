// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include <gtest/gtest.h>

#include "http_client/http_client.h"

using namespace cloud_gateway;

class HttpClientTest : public ::testing::Test {
 protected:
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
