// Yaqoud 2025-2026
// Ahmed Wafdy 2025
// Alaa Hassan 2025
//

#include "mqtt_client.h"

#include <spdlog/spdlog.h>

#include <future>
#include <utility>

cloud_gateway::MqttClient::MqttClient(std::string broker, uint16_t port,
                                      std::string clientId)
    : client_(ioc_, {}, boost::mqtt5::logger(boost::mqtt5::log_level::info)),
      broker_(std::move(broker)),
      port_(port),
      clientId_(std::move(clientId)) {}

// cppcheck-suppress unusedFunction
void cloud_gateway::MqttClient::start_runner() {
  runner_ = std::thread([this]() {
    auto work_guard = boost::asio::make_work_guard(ioc_);

    spdlog::info("Runner thread started.");
    ioc_.run();
    spdlog::info("Runner thread finished.");
  });
}

// cppcheck-suppress unusedFunction
void cloud_gateway::MqttClient::mqtt_connect() {
  client_.brokers(broker_, port_).credentials(clientId_);

  spdlog::info("trying to connect");

  client_.async_run([](const boost::system::error_code& ec) {
    spdlog::error("[MQTT connect Error] {}", ec.message());
  });
}

// cppcheck-suppress unusedFunction
bool cloud_gateway::MqttClient::mqtt_publish(const std::string& topic,
                                             const std::string& payload) {
  std::promise<bool> promise;
  auto fut = promise.get_future();

  spdlog::info("trying to publish");
  client_.async_publish<boost::mqtt5::qos_e::at_most_once>(
      topic, payload, boost::mqtt5::retain_e::yes,
      boost::mqtt5::publish_props{}, [&promise](boost::mqtt5::error_code ec) {
        spdlog::error("[MQTT Publish Error] : {}", ec.message());
        promise.set_value(!ec);
      });

  return fut.get();
}

// cppcheck-suppress unusedFunction
bool cloud_gateway::MqttClient::mqtt_subscribe(const std::string& topic) {
  std::promise<bool> promise;
  auto fut = promise.get_future();

  boost::mqtt5::subscribe_topic sub_topic = boost::mqtt5::subscribe_topic{
      topic,
      boost::mqtt5::subscribe_options{
          boost::mqtt5::qos_e::at_most_once, boost::mqtt5::no_local_e::no,
          boost::mqtt5::retain_as_published_e::retain,
          boost::mqtt5::retain_handling_e::send}};

  client_.async_subscribe(
      sub_topic, boost::mqtt5::subscribe_props{},
      [&promise](boost::mqtt5::error_code ec,
                 const std::vector<boost::mqtt5::reason_code>&,
                 const boost::mqtt5::suback_props& /*props*/) {
        spdlog::error("[MQTT subscribe Error] :{}", ec.message());

        promise.set_value(!ec);
      });

  return fut.get();
}

// cppcheck-suppress unusedFunction
void cloud_gateway::MqttClient::mqtt_disconnect() {
  spdlog::info("trying to disconnect");
  client_.async_disconnect([this](boost::system::error_code ec) {
    spdlog::error("[MQTT disconnect] : {} ", ec.message());
    ioc_.stop();
  });

  if (runner_.joinable()) runner_.join();
}
