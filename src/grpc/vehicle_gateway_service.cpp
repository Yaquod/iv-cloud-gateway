
#include "vehicle_gateway_service.h"

VehicleGatewayServiceImp::VehicleGatewayServiceImp(MqttClient* mqtt, HttpClient* http)
    : mqttClient(mqtt), httpClient(http) {
}

VehicleGatewayServiceImp::~VehicleGatewayServiceImp() = default;

void VehicleGatewayServiceImp::Run(grpc::ServerCompletionQueue* cq) {

    new LoginCallData(&async_service_, cq, httpClient);
    new EtaCallData(&async_service_, cq, mqttClient);
    new StatusCallData(&async_service_, cq, mqttClient);
    new ArriveCallData(&async_service_, cq, mqttClient);

    void* tag;
    bool ok;

    while (true) {
        if (!cq->Next(&tag, &ok)) {
            spdlog::info("Completion queue shutting down");
            break;
        }

        if (ok) {
            static_cast<CallData*>(tag)->Proceed();
        } else {
            spdlog::warn("Call was cancelled");
            delete static_cast<CallData*>(tag);
        }
    }
}

void VehicleGatewayServiceImp::Shutdown()
{
    shutdown_requested = true;
}



VehicleGatewayServiceImp::LoginCallData::LoginCallData
(
    vehicle_gateway::VehicleGateway::AsyncService* service,
    grpc::ServerCompletionQueue* cq,
    HttpClient* http_client)
    : service_(service), cq_(cq), responder_(&ctx_),
      status_(CREATE), http_client_(http_client)
{
    Proceed();
}

void VehicleGatewayServiceImp::LoginCallData::Proceed()
{
    if (status_ == CREATE) {
        status_ = PROCESS;
        service_->RequestVechileLogin(&ctx_, &request_, &responder_, cq_, cq_, this);

    } else if (status_ == PROCESS) {
        new LoginCallData(service_, cq_, http_client_);

        spdlog::info("Processing Login request for VIN: {}", request_.vin_number());

        std::string url = "/api/vechile/login";
        std::string payload = request_.DebugString();

        auto httpResponse = http_client_->Post(url, {}, payload);

        if (!httpResponse.success) {
            response_.set_success(false);
            response_.set_message("HTTP login failed: " + httpResponse.error_message);
            status_ = FINISH;
            responder_.Finish(response_, Status(grpc::StatusCode::INTERNAL, "Http login failed"), this);
        } else {
            response_.set_success(true);
            response_.set_message("login ok");
            status_ = FINISH;
            responder_.Finish(response_, Status::OK, this);
        }

    } else {
        delete this;
    }
}


VehicleGatewayServiceImp::EtaCallData::EtaCallData(
    vehicle_gateway::VehicleGateway::AsyncService* service,
    grpc::ServerCompletionQueue* cq,
    MqttClient* mqtt_client)
    : service_(service), cq_(cq), responder_(&ctx_),
      status_(CREATE), mqtt_client_(mqtt_client) {
    Proceed();
}

void VehicleGatewayServiceImp::EtaCallData::Proceed() {
    if (status_ == CREATE) {
        status_ = PROCESS;
        service_->RequestSendEta(&ctx_, &request_, &responder_, cq_, cq_, this);

    } else if (status_ == PROCESS) {
        new EtaCallData(service_, cq_, mqtt_client_);

        spdlog::info("Processing SendEta request for VIN: {}", request_.vin_number());

        std::string topic = "topic/trip/eta";
        status_ = WAIT_MQTT;


        mqtt_client_->mqtt_publish(topic, request_.DebugString(),
            [this](bool success, std::string message) {

                if (!success) {
                    response_.set_success(false);
                    response_.set_message("mqtt publish Eta failed: " + message);
                    status_ = FINISH;
                    responder_.Finish(response_, Status(grpc::StatusCode::INTERNAL, "Mqtt publish Eta failed"), this);
                } else {
                    response_.set_success(true);
                    response_.set_message("successfully published Eta");
                    status_ = FINISH;
                    responder_.Finish(response_, Status::OK, this);
                }
            });


    } else if (status_ == WAIT_MQTT) {

        status_ = FINISH;

    } else {
        delete this;
    }
}



VehicleGatewayServiceImp::StatusCallData::StatusCallData(
    vehicle_gateway::VehicleGateway::AsyncService* service,
    grpc::ServerCompletionQueue* cq,
    MqttClient* mqtt_client)
    : service_(service), cq_(cq), responder_(&ctx_),
      status_(CREATE), mqtt_client_(mqtt_client) {
    Proceed();
}

void VehicleGatewayServiceImp::StatusCallData::Proceed() {
    if (status_ == CREATE) {
        status_ = PROCESS;
        service_->RequestSendStatus(&ctx_, &request_, &responder_, cq_, cq_, this);

    } else if (status_ == PROCESS) {
        new StatusCallData(service_, cq_, mqtt_client_);

        spdlog::info("Processing SendStatus request for VIN: {}", request_.vin_number());

        std::string topic = "topic/trip/status";
        status_ = WAIT_MQTT;

        mqtt_client_->mqtt_publish(topic, request_.DebugString(),
            [this](bool success, std::string message) {
                if (!success) {
                    response_.set_success(false);
                    response_.set_message("mqtt publish Status failed: " + message);
                    status_ = FINISH;
                    responder_.Finish(response_, Status(grpc::StatusCode::INTERNAL, "Mqtt publish Status failed"), this);
                } else {
                    response_.set_success(true);
                    response_.set_message("successfully published status");
                    status_ = FINISH;
                    responder_.Finish(response_, Status::OK, this);
                }
            });

    } else if (status_ == WAIT_MQTT) {
        status_ = FINISH;

    } else {
        delete this;
    }
}



VehicleGatewayServiceImp::ArriveCallData::ArriveCallData(
    vehicle_gateway::VehicleGateway::AsyncService* service,
    grpc::ServerCompletionQueue* cq,
    MqttClient* mqtt_client)
    : service_(service), cq_(cq), responder_(&ctx_),
      status_(CREATE), mqtt_client_(mqtt_client) {
    Proceed();
}

void VehicleGatewayServiceImp::ArriveCallData::Proceed() {
    if (status_ == CREATE) {
        status_ = PROCESS;
        service_->RequestSendArrive(&ctx_, &request_, &responder_, cq_, cq_, this);

    } else if (status_ == PROCESS) {
        new ArriveCallData(service_, cq_, mqtt_client_);

        spdlog::info("Processing SendArrive request for VIN: {}", request_.vin_number());

        std::string topic = "topic/trip/arrive";
        status_ = WAIT_MQTT;

        mqtt_client_->mqtt_publish(topic, request_.DebugString(),
            [this](bool success, std::string message) {
                if (!success) {
                    response_.set_success(false);
                    response_.set_message("mqtt publish Arrive failed: " + message);
                    status_ = FINISH;
                    responder_.Finish(response_, Status(grpc::StatusCode::INTERNAL, "Mqtt publish Arrive failed"), this);
                } else {
                    response_.set_success(true);
                    response_.set_message("successfully published Arrive");
                    status_ = FINISH;
                    responder_.Finish(response_, Status::OK, this);
                }
            });

    } else if (status_ == WAIT_MQTT) {
        status_ = FINISH;
        
    } else {
        delete this;
    }
}