//
// #include "vehicle_gateway_service.h"
//
// #include <nlohmann/json.hpp>
//
// #include "../infra/constants.h"
//
// VehicleGatewayServiceImp::VehicleGatewayServiceImp(MqttClient* mqtt,
//                                                    HttpClient* http)
//     : mqttClient(mqtt), httpClient(http) {}
//
// VehicleGatewayServiceImp::~VehicleGatewayServiceImp() = default;
//
// void VehicleGatewayServiceImp::Run(grpc::ServerCompletionQueue* cq) {
//   new EtaCallData(&async_service_, cq, mqttClient);
//   new StatusCallData(&async_service_, cq, mqttClient);
//   new ArriveCallData(&async_service_, cq, mqttClient);
//   new UpdateVehicleLocationCallData(&async_service_, cq, mqttClient);
//   new TripInitCallData(&async_service_, cq, mqttClient);
//   new TripMoveCallData(&async_service_, cq, mqttClient);
//
//   void* tag;
//   bool ok;
//
//   while (true) {
//     if (!cq->Next(&tag, &ok)) {
//       spdlog::info("Completion queue shutting down");
//       break;
//     }
//
//     if (ok) {
//       static_cast<CallData*>(tag)->Proceed();
//     } else {
//       spdlog::warn("Call was cancelled");
//       delete static_cast<CallData*>(tag);
//     }
//   }
// }
//
// void VehicleGatewayServiceImp::Shutdown() { shutdown_requested = true; }
//
// VehicleGatewayServiceImp::UpdateVehicleLocationCallData::
//     UpdateVehicleLocationCallData(
//         vehicle_gateway::VehicleGateway::AsyncService* service,
//         grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client)
//     : service_(service),
//       cq_(cq),
//       responder_(&ctx_),
//       status_(CREATE),
//       mqtt_client_(mqtt_client) {
//   Proceed();
// }
//
// void VehicleGatewayServiceImp::UpdateVehicleLocationCallData::Proceed() {
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestUpdateVehicleLocation(&ctx_, &request_, &responder_,
//     cq_,
//                                            cq_, this);
//
//   } else if (status_ == PROCESS) {
//     new UpdateVehicleLocationCallData(service_, cq_, mqtt_client_);
//
//     // modify topic
//     std::string topic = VehicleGatewayConstants::UpdateVehicleLocation;
//     status_ = WAIT_MQTT;
//
//     nlohmann::json body;
//
//     body["vinNumber"] = request_.vinnumber();
//     body["longitude"] = request_.longitude();
//     body["latitude"] = request_.latitude();
//
//     std::string payload = body.dump();
//
//     mqtt_client_->mqtt_publish(
//         topic, payload, [this](bool success, std::string message) {
//           if (!success) {
//             response_.set_success(false);
//             response_.set_message("update vehicle location failed" +
//             message); status_ = FINISH; responder_.Finish(response_,
//                               Status(grpc::StatusCode::INTERNAL,
//                                      "update vehicle location failed"),
//                               this);
//           } else {
//             response_.set_success(true);
//             response_.set_message("successfully updated location");
//             status_ = FINISH;
//             responder_.Finish(response_, Status::OK, this);
//           }
//         });
//
//   } else if (status_ == WAIT_MQTT) {
//     status_ = FINISH;
//
//   } else {
//     delete this;
//   }
// }
//
// VehicleGatewayServiceImp::EtaCallData::EtaCallData(
//     vehicle_gateway::VehicleGateway::AsyncService* service,
//     grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client)
//     : service_(service),
//       cq_(cq),
//       responder_(&ctx_),
//       status_(CREATE),
//       mqtt_client_(mqtt_client) {
//   Proceed();
// }
//
// void VehicleGatewayServiceImp::EtaCallData::Proceed() {
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestSendEta(&ctx_, &request_, &responder_, cq_, cq_, this);
//
//   } else if (status_ == PROCESS) {
//     new EtaCallData(service_, cq_, mqtt_client_);
//
//     std::string topic = VehicleGatewayConstants::TripEta;
//     status_ = WAIT_MQTT;
//
//     nlohmann::json body;
//
//     body["vinNumber"] = request_.vin_number();
//     body["requestId"] = request_.request_id();
//     body["estimatedFare"] = request_.fare();
//     body["estimatedTime"] = request_.time();
//
//     std::string payload = body.dump();
//
//     mqtt_client_->mqtt_publish(
//         topic, payload, [this](bool success, std::string message) {
//           if (!success) {
//             response_.set_success(false);
//             response_.set_message("mqtt publish Eta failed: " + message);
//             status_ = FINISH;
//             responder_.Finish(
//                 response_,
//                 Status(grpc::StatusCode::INTERNAL, "Mqtt publish Eta
//                 failed"), this);
//           } else {
//             response_.set_success(true);
//             response_.set_message("successfully published Eta");
//             status_ = FINISH;
//             responder_.Finish(response_, Status::OK, this);
//           }
//         });
//
//   } else if (status_ == WAIT_MQTT) {
//     status_ = FINISH;
//
//   } else {
//     delete this;
//   }
// }
//
// VehicleGatewayServiceImp::StatusCallData::StatusCallData(
//     vehicle_gateway::VehicleGateway::AsyncService* service,
//     grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client)
//     : service_(service),
//       cq_(cq),
//       responder_(&ctx_),
//       status_(CREATE),
//       mqtt_client_(mqtt_client) {
//   Proceed();
// }
//
// void VehicleGatewayServiceImp::StatusCallData::Proceed() {
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestSendStatus(&ctx_, &request_, &responder_, cq_, cq_,
//     this);
//
//   } else if (status_ == PROCESS) {
//     new StatusCallData(service_, cq_, mqtt_client_);
//
//     std::string topic = VehicleGatewayConstants::TripStatus;
//     status_ = WAIT_MQTT;
//
//     nlohmann::json body;
//
//     body["vinNumber"] = request_.vin_number();
//     body["trip_id"] = request_.trip_id();
//     body["status"] = request_.status();
//
//     std::string payload = body.dump();
//
//     mqtt_client_->mqtt_publish(
//         topic, payload, [this](bool success, std::string message) {
//           if (!success) {
//             response_.set_success(false);
//             response_.set_message("mqtt publish Status failed: " + message);
//             status_ = FINISH;
//             responder_.Finish(response_,
//                               Status(grpc::StatusCode::INTERNAL,
//                                      "Mqtt publish Status failed"),
//                               this);
//           } else {
//             response_.set_success(true);
//             response_.set_message("successfully published status");
//             status_ = FINISH;
//             responder_.Finish(response_, Status::OK, this);
//           }
//         });
//
//   } else if (status_ == WAIT_MQTT) {
//     status_ = FINISH;
//
//   } else {
//     delete this;
//   }
// }
//
// VehicleGatewayServiceImp::ArriveCallData::ArriveCallData(
//     vehicle_gateway::VehicleGateway::AsyncService* service,
//     grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client)
//     : service_(service),
//       cq_(cq),
//       responder_(&ctx_),
//       status_(CREATE),
//       mqtt_client_(mqtt_client) {
//   Proceed();
// }
//
// void VehicleGatewayServiceImp::ArriveCallData::Proceed() {
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestSendArrive(&ctx_, &request_, &responder_, cq_, cq_,
//     this);
//
//   } else if (status_ == PROCESS) {
//     new ArriveCallData(service_, cq_, mqtt_client_);
//
//     std::string topic = VehicleGatewayConstants::TripArrive;
//     status_ = WAIT_MQTT;
//
//     nlohmann::json body;
//
//     body["vinNumber"] = request_.vin_number();
//     body["tripId"] = request_.trip_id();
//     body["longitude"] = request_.long_();
//     body["latitude"] = request_.lat();
//
//     std::string payload = body.dump();
//
//     mqtt_client_->mqtt_publish(
//         topic, payload, [this](bool success, std::string message) {
//           if (!success) {
//             response_.set_success(false);
//             response_.set_message("mqtt publish Arrive failed: " + message);
//             status_ = FINISH;
//             responder_.Finish(response_,
//                               Status(grpc::StatusCode::INTERNAL,
//                                      "Mqtt publish Arrive failed"),
//                               this);
//           } else {
//             response_.set_success(true);
//             response_.set_message("successfully published Arrive");
//             status_ = FINISH;
//             responder_.Finish(response_, Status::OK, this);
//           }
//         });
//
//   } else if (status_ == WAIT_MQTT) {
//     status_ = FINISH;
//
//   } else {
//     delete this;
//   }
// }
//
// VehicleGatewayServiceImp::TripInitCallData::TripInitCallData(
//     vehicle_gateway::VehicleGateway::AsyncService* service,
//     grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client)
//     : service_(service),
//       cq_(cq),
//       responder_(&ctx_),
//       status_(CREATE),
//       mqtt_client_(mqtt_client) {
//   Proceed();
// }
//
// void VehicleGatewayServiceImp::TripInitCallData::Proceed() {
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestTripInit(&ctx_, &request_, &responder_, cq_, cq_, this);
//
//   } else if (status_ == PROCESS) {
//     new TripInitCallData(service_, cq_, mqtt_client_);
//
//     std::string topic = VehicleGatewayConstants::TripInit;
//     status_ = WAIT_MQTT;
//
//     nlohmann::json body;
//
//     body["vinNumber"] = request_.vin_number();
//     body["requestId"] = request_.request_id();
//     body["startLong"] = request_.start_long();
//     body["startLat"] = request_.start_lat();
//     body["endLong"] = request_.end_long();
//     body["endLat"] = request_.end_lat();
//
//     std::string payload = body.dump();
//
//     mqtt_client_->mqtt_publish(
//         topic, payload, [this](bool success, std::string message) {
//           if (!success) {
//             response_.set_success(false);
//             response_.set_message("mqtt publish TripInit failed: " +
//             message); status_ = FINISH; responder_.Finish(response_,
//                               Status(grpc::StatusCode::INTERNAL,
//                                      "Mqtt publish TripInit failed"),
//                               this);
//           } else {
//             response_.set_success(true);
//             response_.set_message("successfully published TripInit");
//             status_ = FINISH;
//             responder_.Finish(response_, Status::OK, this);
//           }
//         });
//
//   } else if (status_ == WAIT_MQTT) {
//     status_ = FINISH;
//
//   } else {
//     delete this;
//   }
// }
//
// VehicleGatewayServiceImp::TripMoveCallData::TripMoveCallData(
//     vehicle_gateway::VehicleGateway::AsyncService* service,
//     grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client)
//     : service_(service),
//       cq_(cq),
//       responder_(&ctx_),
//       status_(CREATE),
//       mqtt_client_(mqtt_client) {
//   Proceed();
// }
//
// void VehicleGatewayServiceImp::TripMoveCallData::Proceed() {
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestTripMove(&ctx_, &request_, &responder_, cq_, cq_, this);
//
//   } else if (status_ == PROCESS) {
//     new TripMoveCallData(service_, cq_, mqtt_client_);
//
//     std::string topic = VehicleGatewayConstants::TripMove;
//     status_ = WAIT_MQTT;
//
//     nlohmann::json body;
//
//     body["vinNumber"] = request_.vin_number();
//     body["tripId"] = request_.trip_id();
//     body["longitude"] = request_.longitude();
//     body["latitude"] = request_.latitude();
//
//     std::string payload = body.dump();
//
//     mqtt_client_->mqtt_publish(
//         topic, payload, [this](bool success, std::string message) {
//           if (!success) {
//             response_.set_success(false);
//             response_.set_message("mqtt publish TripMove failed: " +
//             message); status_ = FINISH; responder_.Finish(response_,
//                               Status(grpc::StatusCode::INTERNAL,
//                                      "Mqtt publish TripMove failed"),
//                               this);
//           } else {
//             response_.set_success(true);
//             response_.set_message("successfully published TripMove");
//             status_ = FINISH;
//             responder_.Finish(response_, Status::OK, this);
//           }
//         });
//
//   } else if (status_ == WAIT_MQTT) {
//     status_ = FINISH;
//
//   } else {
//     delete this;
//   }
// }
