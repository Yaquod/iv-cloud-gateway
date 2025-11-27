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
      mqttClient->mqtt_publish(topic, request->DebugString());


 response->set_success(true);
 response->set_message("Successfully published eta");
 return Status::OK;

}


Status VehicleGatewayServiceImp::SendStatus(ServerContext* context, const vehicle_gateway::StatusRequest* request, vehicle_gateway::StatusResponse* response) {
 std::lock_guard<std::mutex> lock(this->mutex_);
 std::string topic = "topic/trip/status";
 mqttClient->mqtt_publish(topic, request->DebugString());

 response->set_success(true);
 response->set_message("Successfully published status");
 return Status::OK;

}




Status VehicleGatewayServiceImp::SendArrive(ServerContext* context, const vehicle_gateway::ArriveRequest* request, vehicle_gateway::ArriveResponse* response) {
 std::lock_guard<std::mutex> lock(this->mutex_);
 std::string topic = "topic/trip/arrive";
 mqttClient->mqtt_publish(topic, request->DebugString());

 response->set_success(true);
 response->set_message("Successfully published arrive");
 return Status::OK;

}





