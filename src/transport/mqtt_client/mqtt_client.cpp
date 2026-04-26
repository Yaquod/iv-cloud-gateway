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

#include "mqtt_client.h"

#include <spdlog/spdlog.h>

#include <future>
#include <utility>
namespace gateway::transport {
MqttClient::MqttClient(std::string broker, uint16_t port, std::string clientId)
    : ioc_(nullptr),
      runner_(),
      client_(nullptr),
      broker_(std::move(broker)),
      port_(port),
      clientId_(std::move(clientId)),
      started_{false},
      stopped_{false} {}

MqttClient::~MqttClient() { stop(); }

void MqttClient::start() {
  if (started_.exchange(true)) return;
  stopped_ = false;
  if (runner_.joinable()) runner_.join();
  ioc_ = std::make_unique<boost::asio::io_context>();
  client_ = std::unique_ptr<boost::mqtt5::mqtt_client<
      boost::asio::ip::tcp::socket, std::monostate, boost::mqtt5::logger>>(
      new boost::mqtt5::mqtt_client<boost::asio::ip::tcp::socket,
                                    std::monostate, boost::mqtt5::logger>(
          *ioc_, {}, boost::mqtt5::logger(boost::mqtt5::log_level::info)));
  client_->brokers(broker_, port_).credentials(clientId_);
  spdlog::info("[MQTT] trying to connect to broker {}:{}", broker_, port_);
  client_->async_run([](const boost::system::error_code& ec) {
    if (ec) spdlog::error("[MQTT connect Error] {}", ec.message());
  });
  arm_receive();
  runner_ = std::thread([this]() {
    auto guard = boost::asio::make_work_guard(*ioc_);
    ioc_->run();
    spdlog::info("[MQTT] io_context thread exiting");
  });
}

void MqttClient::stop() {
  if (!started_.exchange(false)) return;  // not started
  if (stopped_.exchange(true)) return;    // already stopped
  spdlog::info("[MQTT] trying to disconnect");
  if (client_) {
    client_->async_disconnect([this](boost::system::error_code ec) {
      if (ec) spdlog::error("[MQTT disconnect] : {} ", ec.message());
      if (ioc_) ioc_->stop();
    });
  }
  if (runner_.joinable()) runner_.join();
  client_.reset();
  ioc_.reset();
}

void MqttClient::publish(
    const std::string& topic, const std::string& payload,
    std::function<void(bool success, const std::string msg)> publish_cb) {
  if (!started_ || !client_) {
    if (publish_cb) publish_cb(false, "MQTT client not started");
    return;
  }
  client_->async_publish<boost::mqtt5::qos_e::at_most_once>(
      topic, payload, boost::mqtt5::retain_e::yes,
      boost::mqtt5::publish_props{},
      [topic, publish_cb](boost::mqtt5::error_code ec) {
        if (ec) {
          spdlog::error("[MQTT] failed to publish{} : {}", topic, ec.message());
          if (publish_cb) publish_cb(false, ec.message());
        } else {
          spdlog::info("[MQTT] message publish to {}", topic);
          if (publish_cb) publish_cb(true, "ok");
        }
      });
}

void MqttClient::subscribe(const std::string& topic) {
  if (!started_ || !client_) return;
  boost::mqtt5::subscribe_topic sub_topic = boost::mqtt5::subscribe_topic{
      topic,
      boost::mqtt5::subscribe_options{
          boost::mqtt5::qos_e::at_most_once, boost::mqtt5::no_local_e::no,
          boost::mqtt5::retain_as_published_e::retain,
          boost::mqtt5::retain_handling_e::send}};

  client_->async_subscribe(
      sub_topic, boost::mqtt5::subscribe_props{},
      [this, topic](boost::mqtt5::error_code ec,
                    const std::vector<boost::mqtt5::reason_code>&,
                    const boost::mqtt5::suback_props& /*props*/) {
        if (ec)
          spdlog::error("[MQTT] failed to subscribe to topic {} :{}", topic,
                        ec.message());
        else
          spdlog::info("[MQTT] subscribed to {}", topic);
      });
}

void MqttClient::set_message_handler(MessageHandler handler) {
  cb_ = std::move(handler);
}

void MqttClient::arm_receive() {
  if (!client_) return;
  client_->async_receive([this](boost::mqtt5::error_code ec, std::string topic,
                                std::string payload,
                                boost::mqtt5::publish_props) {
    if (ec) {
      if (!stopped_) {
        spdlog::error("[MQTT] received {} : {}", topic, payload);
        arm_receive();
      }
      return;
    }

    if (cb_) {
      cb_(topic, payload);
    }

    if (!stopped_) {
      arm_receive();
    }
  });
}

}  // namespace gateway::transport
