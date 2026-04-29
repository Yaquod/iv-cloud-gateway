/*
 * Copyright 2026 wafdy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rpc_dispatcher.h"

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include "infra/constants.h"

namespace gateway::services {

void CallDataBase::Proceed() {
  if (state_ == State::CREATE) {
    state_ = State::PROCESS;
    register_rpc();

  } else if (state_ == State::PROCESS) {
    // Spawn a new handler to accept the next call of this type
    clone()->Proceed();

    const std::string topic = mqtt_topic();
    const std::string payload = build_payload();

    state_ = State::FINISH;

    publish_(topic, payload, [this](bool ok, std::string err) {
      if (ok) {
        set_response(true, "ok");
      } else {
        spdlog::error("[RPC] mqtt publish to '{}' failed: {}", mqtt_topic(),
                      err);
        set_response(false, "mqtt publish failed: " + err);
      }
      finish_rpc(ok);
    });

  } else {
    delete this;
  }
}

EtaCallData::EtaCallData(
    vehicle_gateway::VehicleGateway::AsyncService* svc,
    grpc::ServerCompletionQueue* cq,
    std::function<void(const std::string&, const std::string&,
                       std::function<void(bool, std::string)>)>
        publish)
    : CallDataBase(svc, cq, std::move(publish)) {}

void EtaCallData::register_rpc() {
  svc_->RequestSendEta(&ctx_, &request_, &responder_, cq_, cq_, this);
}
std::string EtaCallData::mqtt_topic() const {
  return constants::VehicleGatewayConstants::kTopicTripEta;
}
std::string EtaCallData::build_payload() {
  nlohmann::json j;
  j["vin_number"] = request_.vin_number();
  j["request_id"] = request_.request_id();
  j["estimated_fare"] = request_.fare();
  j["estimated_time"] = request_.time();
  return j.dump();
}
void EtaCallData::set_response(bool ok, const std::string& msg) {
  response_.set_success(ok);
  response_.set_message(msg);
}
void EtaCallData::finish_rpc(bool ok) {
  responder_.Finish(
      response_,
      ok ? grpc::Status::OK
         : grpc::Status(grpc::StatusCode::INTERNAL, response_.message()),
      this);
}
CallDataBase* EtaCallData::clone() {
  return new EtaCallData(svc_, cq_, publish_);
}

StatusCallData::StatusCallData(
    vehicle_gateway::VehicleGateway::AsyncService* svc,
    grpc::ServerCompletionQueue* cq,
    std::function<void(const std::string&, const std::string&,
                       std::function<void(bool, std::string)>)>
        publish)
    : CallDataBase(svc, cq, std::move(publish)) {}

void StatusCallData::register_rpc() {
  svc_->RequestSendStatus(&ctx_, &request_, &responder_, cq_, cq_, this);
}
std::string StatusCallData::mqtt_topic() const {
  return constants::VehicleGatewayConstants::kTopicTripStatus;
}
std::string StatusCallData::build_payload() {
  nlohmann::json j;
  j["vinNumber"] = request_.vin_number();
  j["trip_id"] = request_.trip_id();
  j["status"] = request_.status();
  return j.dump();
}
void StatusCallData::set_response(bool ok, const std::string& msg) {
  response_.set_success(ok);
  response_.set_message(msg);
}
void StatusCallData::finish_rpc(bool ok) {
  responder_.Finish(
      response_,
      ok ? grpc::Status::OK
         : grpc::Status(grpc::StatusCode::INTERNAL, response_.message()),
      this);
}
CallDataBase* StatusCallData::clone() {
  return new StatusCallData(svc_, cq_, publish_);
}

ArriveCallData::ArriveCallData(
    vehicle_gateway::VehicleGateway::AsyncService* svc,
    grpc::ServerCompletionQueue* cq,
    std::function<void(const std::string&, const std::string&,
                       std::function<void(bool, std::string)>)>
        publish)
    : CallDataBase(svc, cq, std::move(publish)) {}

void ArriveCallData::register_rpc() {
  svc_->RequestSendArrive(&ctx_, &request_, &responder_, cq_, cq_, this);
}
std::string ArriveCallData::mqtt_topic() const {
  return constants::VehicleGatewayConstants::kTopicTripArrive;
}
std::string ArriveCallData::build_payload() {
  nlohmann::json j;
  j["vinNumber"] = request_.vin_number();
  j["tripId"] = request_.trip_id();
  j["longitude"] = request_.long_();
  j["latitude"] = request_.lat();
  return j.dump();
}
void ArriveCallData::set_response(bool ok, const std::string& msg) {
  response_.set_success(ok);
  response_.set_message(msg);
}
void ArriveCallData::finish_rpc(bool ok) {
  responder_.Finish(
      response_,
      ok ? grpc::Status::OK
         : grpc::Status(grpc::StatusCode::INTERNAL, response_.message()),
      this);
}
CallDataBase* ArriveCallData::clone() {
  return new ArriveCallData(svc_, cq_, publish_);
}

UpdateLocationCallData::UpdateLocationCallData(
    vehicle_gateway::VehicleGateway::AsyncService* svc,
    grpc::ServerCompletionQueue* cq,
    std::function<void(const std::string&, const std::string&,
                       std::function<void(bool, std::string)>)>
        publish)
    : CallDataBase(svc, cq, std::move(publish)) {}

void UpdateLocationCallData::register_rpc() {
  svc_->RequestUpdateVehicleLocation(&ctx_, &request_, &responder_, cq_, cq_,
                                     this);
}
std::string UpdateLocationCallData::mqtt_topic() const {
  return constants::VehicleGatewayConstants::kTopicUpdateLocation;
}
std::string UpdateLocationCallData::build_payload() {
  nlohmann::json j;
  j["vinNumber"] = request_.vinnumber();
  j["longitude"] = request_.longitude();
  j["latitude"] = request_.latitude();
  return j.dump();
}
void UpdateLocationCallData::set_response(bool ok, const std::string& msg) {
  response_.set_success(ok);
  response_.set_message(msg);
}
void UpdateLocationCallData::finish_rpc(bool ok) {
  responder_.Finish(
      response_,
      ok ? grpc::Status::OK
         : grpc::Status(grpc::StatusCode::INTERNAL, response_.message()),
      this);
}
CallDataBase* UpdateLocationCallData::clone() {
  return new UpdateLocationCallData(svc_, cq_, publish_);
}

TripInitCallData::TripInitCallData(
    vehicle_gateway::VehicleGateway::AsyncService* svc,
    grpc::ServerCompletionQueue* cq,
    std::function<void(const std::string&, const std::string&,
                       std::function<void(bool, std::string)>)>
        publish)
    : CallDataBase(svc, cq, std::move(publish)) {}

void TripInitCallData::register_rpc() {
  svc_->RequestTripInit(&ctx_, &request_, &responder_, cq_, cq_, this);
}
std::string TripInitCallData::mqtt_topic() const {
  return constants::VehicleGatewayConstants::kTopicTripInit;
}
std::string TripInitCallData::build_payload() {
  nlohmann::json j;
  j["vinNumber"] = request_.vin_number();
  j["requestId"] = request_.request_id();
  j["startLong"] = request_.start_long();
  j["startLat"] = request_.start_lat();
  j["endLong"] = request_.end_long();
  j["endLat"] = request_.end_lat();
  return j.dump();
}
void TripInitCallData::set_response(bool ok, const std::string& msg) {
  response_.set_success(ok);
  response_.set_message(msg);
}
void TripInitCallData::finish_rpc(bool ok) {
  responder_.Finish(
      response_,
      ok ? grpc::Status::OK
         : grpc::Status(grpc::StatusCode::INTERNAL, response_.message()),
      this);
}
CallDataBase* TripInitCallData::clone() {
  return new TripInitCallData(svc_, cq_, publish_);
}

TripMoveCallData::TripMoveCallData(
    vehicle_gateway::VehicleGateway::AsyncService* svc,
    grpc::ServerCompletionQueue* cq,
    std::function<void(const std::string&, const std::string&,
                       std::function<void(bool, std::string)>)>
        publish)
    : CallDataBase(svc, cq, std::move(publish)) {}

void TripMoveCallData::register_rpc() {
  svc_->RequestTripMove(&ctx_, &request_, &responder_, cq_, cq_, this);
}
std::string TripMoveCallData::mqtt_topic() const {
  return constants::VehicleGatewayConstants::kTopicTripMove;
}
std::string TripMoveCallData::build_payload() {
  nlohmann::json j;
  j["vinNumber"] = request_.vin_number();
  j["tripId"] = request_.trip_id();
  j["longitude"] = request_.longitude();
  j["latitude"] = request_.latitude();
  return j.dump();
}
void TripMoveCallData::set_response(bool ok, const std::string& msg) {
  response_.set_success(ok);
  response_.set_message(msg);
}
void TripMoveCallData::finish_rpc(bool ok) {
  responder_.Finish(
      response_,
      ok ? grpc::Status::OK
         : grpc::Status(grpc::StatusCode::INTERNAL, response_.message()),
      this);
}
CallDataBase* TripMoveCallData::clone() {
  return new TripMoveCallData(svc_, cq_, publish_);
}

}  // namespace gateway::services