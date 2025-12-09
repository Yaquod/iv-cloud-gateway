#ifndef VEHICLE_GATEWAY_SERVICE_H
#define VEHICLE_GATEWAY_SERVICE_H

#include "http_client/http_client.h"
#include "mqtt_client/mqtt_client.h"
#include <mutex>
#include <spdlog/spdlog.h>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <memory>

#include "../../build/proto/vehicle_gateway.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;
using cloud_gateway::MqttClient;
using cloud_gateway::HttpClient;

class VehicleGatewayServiceImp {
public:
    VehicleGatewayServiceImp(MqttClient* mqtt, HttpClient* http);
    ~VehicleGatewayServiceImp();

    void Run(grpc::ServerCompletionQueue* cq);
    void Shutdown();

    vehicle_gateway::VehicleGateway::AsyncService* GetAsyncService()
    {
        return &async_service_;
    }

private:
    std::mutex mutex_;
    MqttClient* mqttClient;
    HttpClient* httpClient;
    bool shutdown_requested = false;

    vehicle_gateway::VehicleGateway::AsyncService async_service_;

    // Base class for all async call data
    class CallData
    {
    public:
        virtual void Proceed() = 0;
        virtual ~CallData() = default;
    };


    class LoginCallData : public CallData
    {
    public:
        LoginCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
                      grpc::ServerCompletionQueue* cq,
                      HttpClient* http_client);
        void Proceed() override;

    private:
        vehicle_gateway::VehicleGateway::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;
        HttpClient* http_client_;

        vehicle_gateway::LoginRequest request_;
        vehicle_gateway::LoginRespose response_;
        grpc::ServerAsyncResponseWriter<vehicle_gateway::LoginRespose> responder_;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };


    class EtaCallData : public CallData
    {
    public:
        EtaCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
                    grpc::ServerCompletionQueue* cq,
                    MqttClient* mqtt_client);
        void Proceed() override;

    private:
        vehicle_gateway::VehicleGateway::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;
        MqttClient* mqtt_client_;

        vehicle_gateway::EtaRequest request_;
        vehicle_gateway::EtaResponse response_;
        grpc::ServerAsyncResponseWriter<vehicle_gateway::EtaResponse> responder_;

        enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
        CallStatus status_;
    };


    class StatusCallData : public CallData
    {
    public:
        StatusCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
                       grpc::ServerCompletionQueue* cq,
                       MqttClient* mqtt_client);
        void Proceed() override;

    private:
        vehicle_gateway::VehicleGateway::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;
        MqttClient* mqtt_client_;

        vehicle_gateway::StatusRequest request_;
        vehicle_gateway::StatusResponse response_;
        grpc::ServerAsyncResponseWriter<vehicle_gateway::StatusResponse> responder_;

        enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
        CallStatus status_;
    };


    class ArriveCallData : public CallData {
    public:
        ArriveCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
                       grpc::ServerCompletionQueue* cq,
                       MqttClient* mqtt_client);
        void Proceed() override;

    private:
        vehicle_gateway::VehicleGateway::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;
        MqttClient* mqtt_client_;

        vehicle_gateway::ArriveRequest request_;
        vehicle_gateway::ArriveResponse response_;
        grpc::ServerAsyncResponseWriter<vehicle_gateway::ArriveResponse> responder_;

        enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
        CallStatus status_;
    };
};

#endif