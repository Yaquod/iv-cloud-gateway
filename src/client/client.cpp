//
// Created by alaa-hassan on 22‏/11‏/2025.
//

#include "../../build/proto/vehicle_gateway.pb.h"

#include "client.h"

VechileGatewayClient::VechileGatewayClient(const std::string &server_add) {
    auto channel = grpc::CreateChannel(server_add, grpc::InsecureChannelCredentials());
    stub = vehicle_gateway::VehicleGateway::NewStub(channel);

}

void VechileGatewayClient::Login(const std::string &vin_number_val,
            const std::string & trip_id_val

     ) {
    vehicle_gateway::LoginRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);

    vehicle_gateway::LoginRespose resp;
     grpc::ClientContext ctx;
     grpc::Status status = stub->VechileLogin(&ctx, req, &resp);

    spdlog::info("success = {} ,message = {}", resp.success(), resp.message());


}

void VechileGatewayClient::SendEta (
    const std::string &vin_number_val,
    const std::string & trip_id_val,
    double time_val,
    double fare_val
        ) {
    vehicle_gateway::EtaRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_time(time_val);
    req.set_fare(fare_val);

    vehicle_gateway::EtaResponse resp;
    grpc::ClientContext ctx;
    grpc::Status status = stub->SendEta(&ctx, req, &resp);

    //spdlog::error("RPC send eta failed: {}", status.error_message());
    if (status.ok()) {
        spdlog::info("success = {} ,message = {}", resp.success(), resp.message());
    }
    else {
        spdlog::error("RPC send status failed in sendeta: {}", status.error_message());
    }


}


void VechileGatewayClient::SendStatus(
 const std::string &vin_number_val,
 const std::string & trip_id_val,
 const std::string & status_val
     ) {
    vehicle_gateway::StatusRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_status(status_val);

    vehicle_gateway::StatusResponse resp;
    grpc::ClientContext ctx;
    grpc::Status status = stub->SendStatus(&ctx, req, &resp);
    if (status.ok()) {
        spdlog::info("success = {} ,message = {}", resp.success(), resp.message());

    }
    else
    spdlog::error("RPC send status failed in send status: {}", status.error_message());


}

void VechileGatewayClient::SendArrive(
 const std::string &vin_number_val,
 const std::string & trip_id_val,
 double long_val,
 double lat_val
     ) {
    vehicle_gateway::ArriveRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_long_(long_val);
    req.set_lat(lat_val);

    vehicle_gateway::ArriveResponse resp;
    grpc::ClientContext ctx;
    grpc::Status status = stub->SendArrive(&ctx, req, &resp);
    if (status.ok()) {
        spdlog::info("success = {} ,message = {}", resp.success(), resp.message());

    }
    else
        spdlog::error("RPC send status failed in send status: {}", status.error_message());


}

void start_trip_flow() {

    VechileGatewayClient client("localhost:50051");
    client.Login("1234","trip_id1");



}







