#include "gateway.h"

#include "mqtt_topics.h"

cloud_gateway::Gateway::Gateway() :
    httpClient(std::make_unique<HttpClient>()),
    mqttClient(std::make_unique<MqttClient>("broker.hivemq.com", 1883, "boost_mqtt5_test_client")),
    service(std::make_unique<VehicleGatewayServiceImp>(mqttClient.get(), httpClient.get())) {
}

cloud_gateway::Gateway::~Gateway()
{

        shutdown();

}

void cloud_gateway::Gateway::run() {
    std::string server_address("0.0.0.0:50051");
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    builder.RegisterService(service->GetAsyncService());
    cq = builder.AddCompletionQueue();
    server = builder.BuildAndStart();
    if (!server) {
        spdlog::error("[SERVER]Server failed to start!");
        return;
    }

    spdlog::info("[SERVER]Server listening on {}", server_address);

    cq_thread = std::thread(&Gateway::HandleRpcs, this);

    server->Wait();
    if (cq_thread.joinable()) {
        cq_thread.join();
    }
}

void cloud_gateway::Gateway::HandleRpcs()
{
    spdlog::info("[SERVER]Starting RPC handler thread");
    service->Run(cq.get());
    spdlog::info("[SERVER]RPC handler thread exiting");
}

void cloud_gateway::Gateway::shutdown() {



    spdlog::info("[SERVER]Gateway shutdown initiated");

    if (cq) {
        cq->Shutdown();
    }

    if (server) {
        server->Shutdown();
    }
    

    
    if (mqttClient) {
        mqttClient->mqtt_disconnect();
    }
    
    if (service) {
        service->Shutdown();
    }
}

void cloud_gateway::Gateway::initialize() {
    mqttClient->start_runner();
    mqttClient->mqtt_connect();
    mqttClient->mqtt_subscribe(MqttTopics::TripInit);
    mqttClient->mqtt_subscribe(MqttTopics::TripMove);

    mqttClient->set_message_arrived_handler([](const std::string &topic, const std::string &payload) {

        on_mqtt_message(topic , payload);
    });
    mqttClient->start_receive_loop();
}

