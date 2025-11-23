//
// Created by alaa-hassan on 19‏/11‏/2025.
//

#ifndef VEHICLE_GATEWAY_SERVICE_H
#define VEHICLE_GATEWAY_SERVICE_H


#include <grpcpp/server.h>
#include <grpcpp/server_context.h>
#include "vehicle_gateway.grpc.pb.h"
#include "http_client/http_client.h"
#include "mqtt_client/mqtt_client.h"
#include <mutex>
#include <spdlog/spdlog.h>

using grpc::Status;
using grpc::ServerContext;
using cloud_gateway::MqttClient;
using cloud_gateway::HttpClient;

class VehicleGatewayServiceImp final :public vehicle_gateway::VehicleGateway::Service {
public:
    VehicleGatewayServiceImp (MqttClient* mqtt , HttpClient* http);
    ~VehicleGatewayServiceImp () ;
    Status VechileLogin(ServerContext* context, const vehicle_gateway::LoginRequest* request, vehicle_gateway::LoginRespose* response)override;
    Status SendEta(ServerContext* context, const vehicle_gateway::EtaRequest* request, vehicle_gateway::EtaResponse* response)override;
    Status SendStatus(ServerContext* context, const vehicle_gateway::StatusRequest* request, vehicle_gateway::StatusResponse* response)override;
    Status ReceiveCommand(ServerContext* context, const vehicle_gateway::CommandRequest* request, vehicle_gateway::CommandResponse* response)override;


private:
    std::mutex mutex_;
    MqttClient* mqttClient;
    HttpClient* httpClient;



};
#endif