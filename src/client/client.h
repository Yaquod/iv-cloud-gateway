#ifndef YAQOUD_PROJECT_CLIENT_H
#define YAQOUD_PROJECT_CLIENT_H
#include <grpcpp/grpcpp.h>
#include "vehicle_gateway.grpc.pb.h"
#include <string>
#include <spdlog/spdlog.h>
#include <thread>
#include <condition_variable>
#include <atomic>


void start_trip_flow();

class VechileGatewayClient
{
public:
    VechileGatewayClient(const std::string &server_add);
    ~VechileGatewayClient();
    
    bool Login(const std::string &vin_number_val, const std::string &trip_id_val);
    bool SendEta(const std::string &vin_number_val, const std::string &trip_id_val, double time_val, double fare_val);
    bool SendStatus(const std::string &vin_number_val, const std::string &trip_id_val, const std::string &status_val);
    bool SendArrive(const std::string &vin_number_val, const std::string &trip_id_val, double long_val, double lat_val);

private:
    std::unique_ptr<vehicle_gateway::VehicleGateway::Stub> stub;
    grpc::CompletionQueue cq;
    std::thread cq_thread;
    std::atomic<bool> shutdown_requested{false};
    
    void ProcessCompletionQueue();
    

    struct AsyncCallDataBase
    {
        grpc::ClientContext ctx;
        grpc::Status status;
        bool done = false;
        bool result = false;
        std::mutex mu;
        std::condition_variable cv;
        virtual ~AsyncCallDataBase() = default;
    };
    

    struct LoginCallData : AsyncCallDataBase
    {
        vehicle_gateway::LoginRespose response;
        std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::LoginRespose>> response_reader;
    };
    
    struct EtaCallData : AsyncCallDataBase {
        vehicle_gateway::EtaResponse response;
        std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::EtaResponse>> response_reader;
    };
    
    struct StatusCallData : AsyncCallDataBase {
        vehicle_gateway::StatusResponse response;
        std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::StatusResponse>> response_reader;
    };
    
    struct ArriveCallData : AsyncCallDataBase {
        vehicle_gateway::ArriveResponse response;
        std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::ArriveResponse>> response_reader;
    };
};

#endif //YAQOUD_PROJECT_CLIENT_H
