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
    mqttClient->mqtt_subscribe("topic/trip/init");
    mqttClient->mqtt_subscribe("topic/trip/move");


    mqttClient->set_message_arrived_handler([](const std::string &topic , const std::string &payload) {
        parsing_received_command(topic, payload);
    });
    mqttClient->start_receive_loop();
}

void cloud_gateway::parsing_received_command(const std::string& topic, const std::string& payload) {
    if (topic == "topic/trip/init")
    {
        spdlog::info("received message on init topic");
    }
    else if (topic == "topic/trip/move")
    {
        spdlog::info("received message on move topic");
    }
    else {
        spdlog::info("undefined topic");
    }
}








// TODO: make gateway as lib to be used by other projects