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
#pragma once

#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>

#include "infra/constants.h"
#include "services/stream_tag.h"
#include "vehicle_gateway.grpc.pb.h"

namespace gateway::services {

class VehicleStreamHandler;

class StreamWriter : public IStreamTag {
 public:
  explicit StreamWriter(VehicleStreamHandler* owner) : owner_(owner) {}
  void Proceed(bool ok) override;

 private:
  VehicleStreamHandler* owner_;
};

class VehicleStreamHandler : public IStreamTag {
 public:
  using PublishFn = std::function<void(const std::string&, const std::string&,
                                       std::function<void(bool, std::string)>)>;

  VehicleStreamHandler(vehicle_gateway::VehicleGateway::AsyncService* svc,
                       grpc::ServerCompletionQueue* cq, PublishFn publish)
      : svc_(svc),
        cq_(cq),
        publish_(std::move(publish)),
        stream_(&ctx_),
        state_(State::CREATE),
        writing_(false),
        writer_(this) {
    Proceed(true);
  }

  void Proceed(bool ok) override {
    switch (state_) {
      case State::CREATE:
        state_ = State::ACCEPTING;
        svc_->RequestVehicleCommandStream(&ctx_, &stream_, cq_, cq_, this);
        break;

      case State::ACCEPTING:
        if (!ok) {
          delete this;
          return;
        }
        spdlog::info("[Stream] Autoware connected — active_ set");
        new VehicleStreamHandler(svc_, cq_,
                                 publish_);  // sibling for next connection
        active_.store(this);
        flush_pending(this);
        state_ = State::READING;
        stream_.Read(&incoming_, this);
        break;

      case State::READING:
        if (!ok) {
          spdlog::warn("[Stream] Autoware disconnected");
          {
            VehicleStreamHandler* exp = this;
            active_.compare_exchange_strong(exp, nullptr);
          }
          state_ = State::FINISH;
          stream_.Finish(grpc::Status::OK, this);
          return;
        }
        handle_incoming(incoming_);
        stream_.Read(&incoming_, this);
        break;

      case State::FINISH: {
        VehicleStreamHandler* exp = this;
        active_.compare_exchange_strong(exp, nullptr);
      }
        delete this;
        break;
    }
  }

  // Called by StreamWriter tag when a Write() completes
  void on_write_done(bool ok) {
    if (!ok) {
      spdlog::warn("[Stream] Write failed");
      writing_.store(false);
      return;
    }
    std::lock_guard<std::mutex> lk(mu_);
    writing_.store(false);
    flush_write_queue_locked();
  }

  void enqueue(const vehicle_gateway::GatewayCommand& cmd) {
    std::lock_guard<std::mutex> lk(mu_);
    write_queue_.push(cmd);
    if (!writing_.load()) flush_write_queue_locked();
  }

  static void push_command(const vehicle_gateway::GatewayCommand& cmd) {
    auto* h = active_.load();
    if (!h) {
      spdlog::warn("[Stream] No Autoware connected — queuing command");
      std::lock_guard<std::mutex> lk(pending_mu_);
      pending_commands_.push(cmd);
      return;
    }
    h->enqueue(cmd);
  }

  static std::atomic<VehicleStreamHandler*> active_;
  static std::queue<vehicle_gateway::GatewayCommand> pending_commands_;
  static std::mutex pending_mu_;

 private:
  enum class State { CREATE, ACCEPTING, READING, FINISH };

  void flush_write_queue_locked() {
    if (write_queue_.empty() || writing_.load()) return;
    writing_.store(true);
    stream_.Write(write_queue_.front(), &writer_);
    write_queue_.pop();
  }

  static void flush_pending(VehicleStreamHandler* h) {
    std::lock_guard<std::mutex> lk(pending_mu_);
    int n = 0;
    while (!pending_commands_.empty()) {
      h->enqueue(pending_commands_.front());
      pending_commands_.pop();
      ++n;
    }
    if (n) spdlog::info("[Stream] Flushed {} pending command(s)", n);
  }

  void handle_incoming(const vehicle_gateway::VehicleEvent& ev) {
    if (ev.has_trip_init_ack()) {
      spdlog::info("[Stream] TripInitAck success={}",
                   ev.trip_init_ack().success());
    } else if (ev.has_eta()) {
      auto& r = ev.eta();
      nlohmann::json j;
      j["vin_number"] = r.vin_number();
      j["request_id"] = r.request_id();
      j["estimated_fare"] = r.fare();
      j["estimated_time"] = r.time();
      publish_(constants::VehicleGatewayConstants::kTopicTripEta, j.dump(),
               [](bool ok, std::string e) {
                 if (!ok) spdlog::error("[Stream] ETA failed: {}", e);
               });
    } else if (ev.has_status()) {
      auto& r = ev.status();
      nlohmann::json j;
      j["vinNumber"] = r.vin_number();
      j["trip_id"] = r.trip_id();
      j["status"] = r.status();
      publish_(constants::VehicleGatewayConstants::kTopicTripStatus, j.dump(),
               [](bool ok, std::string e) {
                 if (!ok) spdlog::error("[Stream] Status failed: {}", e);
               });
    } else if (ev.has_arrive()) {
      auto& r = ev.arrive();
      nlohmann::json j;
      j["vinNumber"] = r.vin_number();
      j["tripId"] = r.trip_id();
      j["longitude"] = r.long_();
      j["latitude"] = r.lat();
      publish_(constants::VehicleGatewayConstants::kTopicTripArrive, j.dump(),
               [](bool ok, std::string e) {
                 if (!ok) spdlog::error("[Stream] Arrive failed: {}", e);
               });
    } else if (ev.has_location()) {
      auto& r = ev.location();
      nlohmann::json j;
      j["vinNumber"] = r.vinnumber();
      j["latitude"] = r.latitude();
      j["longitude"] = r.longitude();
      publish_(constants::VehicleGatewayConstants::kTopicUpdateLocation,
               j.dump(), [](bool ok, std::string e) {
                 if (!ok) spdlog::error("[Stream] Loc failed: {}", e);
               });
    }
  }

  vehicle_gateway::VehicleGateway::AsyncService* svc_;
  grpc::ServerCompletionQueue* cq_;
  PublishFn publish_;
  grpc::ServerContext ctx_;
  grpc::ServerAsyncReaderWriter<vehicle_gateway::GatewayCommand,
                                vehicle_gateway::VehicleEvent>
      stream_;
  vehicle_gateway::VehicleEvent incoming_;
  State state_;
  std::mutex mu_;
  std::queue<vehicle_gateway::GatewayCommand> write_queue_;
  std::atomic<bool> writing_;
  StreamWriter writer_;
};

inline void StreamWriter::Proceed(bool ok) { owner_->on_write_done(ok); }

}  // namespace gateway::services