// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//

#include "mqtt_client.h"

#include <future>
#include <iostream>

cloud_gateway::MqttClient::MqttClient(const std::string& broker, uint16_t port,
                                      const std::string& clientId)
    : client_(ioc_, {}, boost::mqtt5::logger(boost::mqtt5::log_level::info)),
      broker_(broker),
      port_(port),
      clientId_(clientId) {}

// cppcheck-suppress unusedFunction
void cloud_gateway::MqttClient::start_runner() {
  runner_ = std::thread([this]() {
    auto work_guard = boost::asio::make_work_guard(ioc_);

    std::cerr << "Runner thread started." << std::endl;
    ioc_.run();
    std::cerr << "Runner thread finished." << std::endl;
  });
}

// cppcheck-suppress unusedFunction
void cloud_gateway::MqttClient::mqtt_connect() {
  client_.brokers(broker_, port_).credentials(clientId_);

  std::cerr << "trying to connect" << std::endl;

  client_.async_run([](const boost::system::error_code& ec) {
    std::cerr << "[MQTT connect Error] " << ec.message() << std::endl;
  });
}

// cppcheck-suppress unusedFunction
bool cloud_gateway::MqttClient::mqtt_publish(const std::string& topic,
                                             const std::string& payload) {
  std::promise<bool> promise;
  auto fut = promise.get_future();

  std::cerr << "trying to publish" << std::endl;
  client_.async_publish<boost::mqtt5::qos_e::at_most_once>(
      topic, payload, boost::mqtt5::retain_e::yes,
      boost::mqtt5::publish_props{}, [&promise](boost::mqtt5::error_code ec) {
        std::cerr << "[MQTT Publish Error] " << ec.message() << std::endl;
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

  client_.async_subscribe(sub_topic, boost::mqtt5::subscribe_props{},
                          [&promise](boost::mqtt5::error_code ec,
                                     std::vector<boost::mqtt5::reason_code>,
                                     boost::mqtt5::suback_props /*props*/) {
                            std::cerr << "[MQTT subscribe Error] "
                                      << ec.message() << std::endl;

                            promise.set_value(!ec);
                          });

  return fut.get();
}

// cppcheck-suppress unusedFunction
void cloud_gateway::MqttClient::mqtt_disconnect() {
  std::cerr << "trying to disconnect" << std::endl;
  client_.async_disconnect([this](boost::system::error_code ec) {
    std::cerr << "[MQTT disconnect] " << ec.message() << std::endl;
    ioc_.stop();
  });

  if (runner_.joinable()) runner_.join();
}
