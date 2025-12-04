//
// Created by alaa-hassan on 19‏/11‏/2025.
//

#ifndef VEHICLE_GATEWAY_SERVICE_H
#define VEHICLE_GATEWAY_SERVICE_H



#include "http_client/http_client.h"
#include "mqtt_client/mqtt_client.h"
#include <mutex>
#include <spdlog/spdlog.h>



#include "../../build/proto/vehicle_gateway.grpc.pb.h"


using grpc::Status;
using grpc::ServerContext;
using cloud_gateway::MqttClient;
using cloud_gateway::HttpClient;

class VehicleGatewayServiceImp final :public vehicle_gateway::VehicleGateway::ExperimentalCallbackService
{
public:
    VehicleGatewayServiceImp (MqttClient* mqtt , HttpClient* http);
    ~VehicleGatewayServiceImp () ;
    grpc::experimental::ServerUnaryReactor* VechileLogin(grpc::experimental::CallbackServerContext* context, const vehicle_gateway::LoginRequest* request, vehicle_gateway::LoginRespose* response)override;
    grpc::experimental::ServerUnaryReactor* SendEta(grpc::experimental::CallbackServerContext* context, const vehicle_gateway::EtaRequest* request, vehicle_gateway::EtaResponse* response)override;
    grpc::experimental::ServerUnaryReactor* SendStatus(grpc::experimental::CallbackServerContext* context, const vehicle_gateway::StatusRequest* request, vehicle_gateway::StatusResponse* response)override;
    grpc::experimental::ServerUnaryReactor* SendArrive(grpc::experimental::CallbackServerContext* context, const vehicle_gateway::ArriveRequest* request, vehicle_gateway::ArriveResponse* response)override;


private:
    std::mutex mutex_;
    MqttClient* mqttClient;
    HttpClient* httpClient;



};
#endif