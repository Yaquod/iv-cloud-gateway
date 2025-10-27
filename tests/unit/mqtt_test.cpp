//
// Created by ahmed on 10/27/25.
//

#include <gtest/gtest.h>

#include "mqtt_client/mqtt_client.h"

class MqttClientTest : public ::testing::Test {
 protected:
  cloud_gateway::MqttClient MqttClient;
};

TEST_F(MqttClientTest, PublishRequestTest) {}
