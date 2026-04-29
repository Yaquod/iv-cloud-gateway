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

#ifndef VEHICLECLOUDGATEWAY_RPC_DISPATCHER_H
#define VEHICLECLOUDGATEWAY_RPC_DISPATCHER_H

#pragma once

#include <vehicle_gateway.grpc.pb.h>

#include <functional>
#include <string>

namespace gateway::services {
class CallDataBase {
 public:
  virtual ~CallDataBase() = default;
  void Proceed();

 protected:
  enum class State { CREATE, PROCESS, FINISH };

  explicit CallDataBase(
      vehicle_gateway::VehicleGateway::AsyncService* svc,
      grpc::ServerCompletionQueue* cq,
      std::function<void(const std::string& topic, const std::string& payload,
                         std::function<void(bool, std::string)> cb)>
          publish)
      : svc_(svc),
        cq_(cq),
        publish_(std::move(publish)),
        state_(State::CREATE) {}

  vehicle_gateway::VehicleGateway::AsyncService* svc_;
  grpc::ServerCompletionQueue* cq_;
  grpc::ServerContext ctx_;

  std::function<void(const std::string&, const std::string&,
                     std::function<void(bool, std::string)>)>
      publish_;
  State state_;

  virtual void register_rpc() = 0;
  virtual std::string mqtt_topic() const = 0;
  virtual std::string build_payload() = 0;
  virtual void set_response(bool ok, const std::string& msg) = 0;
  virtual void finish_rpc(bool ok) = 0;
  virtual CallDataBase* clone() = 0;
};

class EtaCallData final : public CallDataBase {
 public:
  EtaCallData(vehicle_gateway::VehicleGateway::AsyncService* svc,
              grpc::ServerCompletionQueue* cq,
              std::function<void(const std::string&, const std::string&,
                                 std::function<void(bool, std::string)>)>
                  publish);

 private:
  void register_rpc() override;
  std::string mqtt_topic() const override;
  std::string build_payload() override;
  void set_response(bool ok, const std::string& msg) override;
  void finish_rpc(bool ok) override;
  CallDataBase* clone() override;

  vehicle_gateway::EtaRequest request_;
  vehicle_gateway::EtaResponse response_;
  grpc::ServerAsyncResponseWriter<vehicle_gateway::EtaResponse> responder_{
      &ctx_};
};

class StatusCallData final : public CallDataBase {
 public:
  StatusCallData(vehicle_gateway::VehicleGateway::AsyncService* svc,
                 grpc::ServerCompletionQueue* cq,
                 std::function<void(const std::string&, const std::string&,
                                    std::function<void(bool, std::string)>)>
                     publish);

 private:
  void register_rpc() override;
  std::string mqtt_topic() const override;
  std::string build_payload() override;
  void set_response(bool ok, const std::string& msg) override;
  void finish_rpc(bool ok) override;
  CallDataBase* clone() override;

  vehicle_gateway::StatusRequest request_;
  vehicle_gateway::StatusResponse response_;
  grpc::ServerAsyncResponseWriter<vehicle_gateway::StatusResponse> responder_{
      &ctx_};
};

class ArriveCallData final : public CallDataBase {
 public:
  ArriveCallData(vehicle_gateway::VehicleGateway::AsyncService* svc,
                 grpc::ServerCompletionQueue* cq,
                 std::function<void(const std::string&, const std::string&,
                                    std::function<void(bool, std::string)>)>
                     publish);

 private:
  void register_rpc() override;
  std::string mqtt_topic() const override;
  std::string build_payload() override;
  void set_response(bool ok, const std::string& msg) override;
  void finish_rpc(bool ok) override;
  CallDataBase* clone() override;

  vehicle_gateway::ArriveRequest request_;
  vehicle_gateway::ArriveResponse response_;
  grpc::ServerAsyncResponseWriter<vehicle_gateway::ArriveResponse> responder_{
      &ctx_};
};

class UpdateLocationCallData final : public CallDataBase {
 public:
  UpdateLocationCallData(
      vehicle_gateway::VehicleGateway::AsyncService* svc,
      grpc::ServerCompletionQueue* cq,
      std::function<void(const std::string&, const std::string&,
                         std::function<void(bool, std::string)>)>
          publish);

 private:
  void register_rpc() override;
  std::string mqtt_topic() const override;
  std::string build_payload() override;
  void set_response(bool ok, const std::string& msg) override;
  void finish_rpc(bool ok) override;
  CallDataBase* clone() override;

  vehicle_gateway::UpdateVehicleLocationRequest request_;
  vehicle_gateway::UpdateVehicleLocationResponse response_;
  grpc::ServerAsyncResponseWriter<
      vehicle_gateway::UpdateVehicleLocationResponse>
      responder_{&ctx_};
};

class TripInitCallData final : public CallDataBase {
 public:
  TripInitCallData(vehicle_gateway::VehicleGateway::AsyncService* svc,
                   grpc::ServerCompletionQueue* cq,
                   std::function<void(const std::string&, const std::string&,
                                      std::function<void(bool, std::string)>)>
                       publish);

 private:
  void register_rpc() override;
  std::string mqtt_topic() const override;
  std::string build_payload() override;
  void set_response(bool ok, const std::string& msg) override;
  void finish_rpc(bool ok) override;
  CallDataBase* clone() override;

  vehicle_gateway::TripInitRequest request_;
  vehicle_gateway::TripInitResponse response_;
  grpc::ServerAsyncResponseWriter<vehicle_gateway::TripInitResponse> responder_{
      &ctx_};
};

class TripMoveCallData final : public CallDataBase {
 public:
  TripMoveCallData(vehicle_gateway::VehicleGateway::AsyncService* svc,
                   grpc::ServerCompletionQueue* cq,
                   std::function<void(const std::string&, const std::string&,
                                      std::function<void(bool, std::string)>)>
                       publish);

 private:
  void register_rpc() override;
  std::string mqtt_topic() const override;
  std::string build_payload() override;
  void set_response(bool ok, const std::string& msg) override;
  void finish_rpc(bool ok) override;
  CallDataBase* clone() override;

  vehicle_gateway::TripMoveRequest request_;
  vehicle_gateway::TripMoveResponse response_;
  grpc::ServerAsyncResponseWriter<vehicle_gateway::TripMoveResponse> responder_{
      &ctx_};
};

}  // namespace gateway::services
#endif  // VEHICLECLOUDGATEWAY_RPC_DISPATCHER_H
