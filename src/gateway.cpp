// Yaqoud 2025-2026
// Ahmed Wafdy 2025
//
#include "gateway.h"



cloud_gateway::Gateway::Gateway() :
httpClient(std::make_unique<HttpClient>()),
mqttClient(std::make_unique<MqttClient>("broker.hivemq.com", 1883, "boost_mqtt5_test_client")),
service(std::make_unique<VehicleGatewayServiceImp>(mqttClient.get() , httpClient.get()))
{

}


// cloud_gateway::Gateway::~Gateway() {
//
// }


void cloud_gateway::Gateway::run() {

    std::string server_address("0.0.0.0:50051");
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(service.get());
    server = builder.BuildAndStart();
    spdlog::info("server start response");
    server->Wait();

}

void cloud_gateway::Gateway::shutdown() {
    if (server)server->Shutdown();
    if (mqttClient)mqttClient->mqtt_disconnect();
}

void cloud_gateway::Gateway::initialize() {

    mqttClient->start_runner();
    mqttClient->mqtt_connect();
    //subscribe to the topic
}





// TODO: make gateway as lib to be used by other projects