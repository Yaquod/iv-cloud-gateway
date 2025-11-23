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
            const std::string & trip_id_val,
             double starting_long_val,
             double starting_lat_val,
             double ending_lon_val,
             double ending_lat_val
     ) {
    vehicle_gateway::LoginRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_starting_long(starting_long_val);
    req.set_starting_lat(starting_lat_val);
    req.set_ending_long(ending_lon_val);
    req.set_ending_lat(ending_lat_val);

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

    spdlog::error("RPC send eta failed: {}", status.error_message());
    spdlog::info("success = {} ,message = {}", resp.success(), resp.message());


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
        //spdlog::info("server respond ok ");
    }
    else
    spdlog::error("RPC send status failed: {}", status.error_message());
     //spdlog::info("success = {} ,message = {}", resp.success(), resp.message());

}







