//
// Created by alaa-hassan on 31‏/10‏/2025.
//

#include <gtest/gtest.h>

#include "../../src/mqtt_client/mqtt_client.h"

using namespace cloud_gateway;

class MqttClientTest : public ::testing::Test {
 protected:
  MqttClient client{"broker.hivemq.com", 1883, "boost_mqtt5_test_client"};
};

TEST_F(MqttClientTest, PublishTest) {
  client.start_runner();
  client.mqtt_connect();
  bool res = client.mqtt_publish("mqtt5/test", "hello");
  client.mqtt_disconnect();
  EXPECT_TRUE(res);
}

TEST_F(MqttClientTest, SubscribeTest) {
  client.start_runner();
  client.mqtt_connect();
  bool res = client.mqtt_subscribe("mqtt5/test");
  client.mqtt_disconnect();
  EXPECT_TRUE(res);
}
