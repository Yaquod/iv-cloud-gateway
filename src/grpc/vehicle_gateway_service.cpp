//
// Created by alaa-hassan on 19‏/11‏/2025.
//
#include "vehicle_gateway_service.h"


VehicleGatewayServiceImp::VehicleGatewayServiceImp(MqttClient* mqtt,HttpClient* http)
 : mqttClient(mqtt),httpClient(http){

}

VehicleGatewayServiceImp::~VehicleGatewayServiceImp() = default;

Status VehicleGatewayServiceImp::VechileLogin(ServerContext *context, const vehicle_gateway::LoginRequest *request,
                                              vehicle_gateway::LoginRespose *response) {
 std::lock_guard<std::mutex> lock(this->mutex_);
 //spdlog::info("Login request from VIN: {}", request->vin());
 std::string url = "/api/vechile/login";
 std::string payload = request->DebugString();

 auto httpResponse = httpClient->Post(url, {}, payload);
 if (!httpResponse.success) {
  response->set_success(false);
  response->set_message("HTTP login failed: " + httpResponse.error_message);
  return Status(grpc::StatusCode::INTERNAL, "HTTP login failed");
 }

 response->set_success(true);
 response->set_message("login ok");
 return Status::OK;
}

Status VehicleGatewayServiceImp::SendEta(ServerContext* context, const vehicle_gateway::EtaRequest* request, vehicle_gateway::EtaResponse* response) {
      std::lock_guard<std::mutex> lock(this->mutex_);
      std::string topic = "topic/trip/eta";
      bool ok = mqttClient->mqtt_publish(topic, request->DebugString());
 if (!ok) {
  response->set_success(false);
  response->set_message("Failed to publish eta");
  return Status(grpc::StatusCode::INTERNAL ,"Failed to publish eta");
 }
 response->set_success(true);
 response->set_message("Successfully published eta");
 return Status::OK;

}


Status VehicleGatewayServiceImp::SendStatus(ServerContext* context, const vehicle_gateway::StatusRequest* request, vehicle_gateway::StatusResponse* response) {
 std::lock_guard<std::mutex> lock(this->mutex_);
 std::string topic = "topic/trip/status";
 bool ok = mqttClient->mqtt_publish(topic, request->DebugString());
 spdlog::info("Received SendStatus from VIN: {}", request->vin_number());
 if (!ok) {
  //response->set_success(false);
  //response->set_message("Failed to publish status");
  return Status(grpc::StatusCode::INTERNAL ,"Failed to publish status");
 }
 response->set_success(true);
 response->set_message("Successfully published status");
 return Status::OK;

}

Status VehicleGatewayServiceImp::ReceiveCommand(ServerContext *context, const vehicle_gateway::CommandRequest *request, vehicle_gateway::CommandResponse *response)
{

 return Status::OK;
}





