#ifndef GATEWAY_H
#define GATEWAY_H

#include <memory>
#include <thread>

#include "http_client/http_client.h"
#include "mqtt_client/mqtt_client.h"
#include "grpc/vehicle_gateway_service.h"
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_builder.h>
#include <spdlog/spdlog.h>

namespace cloud_gateway {

 void parsing_received_command(const std::string& topic, const std::string& payload);

 class Gateway {
 public:
  Gateway();
  ~Gateway();

  void initialize();
  void run();
  void shutdown();

 private:
  std::unique_ptr<HttpClient> httpClient;
  std::unique_ptr<MqttClient> mqttClient;
  std::unique_ptr<grpc::Server> server;
  std::unique_ptr<VehicleGatewayServiceImp> service;
  std::unique_ptr<grpc::ServerCompletionQueue> cq;
  std::thread cq_thread;


  void HandleRpcs();
 };

}  // namespace cloud_gateway

#endif