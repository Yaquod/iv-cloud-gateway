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

#include <gtest/gtest.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include "../../../src/transport/mqtt_client/mqtt_client.h"

using namespace gateway::transport;

class MqttClientTest : public ::testing::Test {
 protected:
  MqttClient client{"broker.hivemq.com", 1883, "test_client_id"};
  void SetUp() override {
    client.start();
  }
  void TearDown() override {
    client.stop();
  }
};

TEST_F(MqttClientTest, ConnectDisconnect) {
  // Should connect and disconnect without error
  client.stop();
  client.start();
  client.stop();
  SUCCEED();
}

TEST_F(MqttClientTest, PublishSuccess) {
  std::atomic<bool> called{false};
  client.publish("mqtt5/test", "hello-mqtt", [&](bool success, const std::string& msg) {
    EXPECT_TRUE(success);
    called = true;
  });
  for (int i = 0; i < 20 && !called; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(called);
}

TEST_F(MqttClientTest, SubscribeSuccess) {
  // Should not throw or log error
  client.subscribe("mqtt5/test");
  SUCCEED();
}

TEST_F(MqttClientTest, MessageArrivalCallback) {
  std::mutex mtx;
  std::condition_variable cv;
  bool received = false;
  client.set_message_handler([&](const std::string& topic, const std::string& payload) {
    std::lock_guard<std::mutex> lock(mtx);
    received = true;
    cv.notify_one();
  });
  client.subscribe("mqtt5/test");
  client.publish("mqtt5/test", "test-payload", nullptr);
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait_for(lock, std::chrono::seconds(5), [&] { return received; });
  EXPECT_TRUE(received);
}

TEST_F(MqttClientTest, PublishFailureInvalidTopic) {
  std::atomic<bool> called{false};
  client.publish("", "payload", [&](bool success, const std::string& msg) {
    EXPECT_FALSE(success);
    called = true;
  });
  for (int i = 0; i < 20 && !called; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(called);
}

TEST_F(MqttClientTest, SubscribeFailureInvalidTopic) {
  client.subscribe("");
  SUCCEED();
}

TEST_F(MqttClientTest, DoubleStartStop) {
  client.start();
  client.start();
  client.stop();
  client.stop();
  SUCCEED();
}

TEST_F(MqttClientTest, PublishBeforeStart) {
  MqttClient temp_client{"broker.hivemq.com", 1883, "test_client_id2"};
  std::atomic<bool> called{false};
  temp_client.publish("mqtt5/test", "payload", [&](bool success, const std::string& msg) {
    EXPECT_FALSE(success);
    called = true;
  });
  for (int i = 0; i < 20 && !called; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(called);
}

TEST_F(MqttClientTest, SubscribeBeforeStart) {
  MqttClient temp_client{"broker.hivemq.com", 1883, "test_client_id3"};
  temp_client.subscribe("mqtt5/test");
  SUCCEED();
}
