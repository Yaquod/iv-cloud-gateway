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
#include <set>

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
        spdlog::info("[Stream] client connected");
        new VehicleStreamHandler(svc_, cq_,
                                 publish_);  // sibling for next connection
        // Both autoware_agent (the vehicle bridge) and flutter-ivi (the
        // driver-facing app) connect to this same stream independently and
        // are both meant to observe every GatewayCommand. Track the full set
        // of connected clients rather than a single "active" one -- a single
        // pointer meant only one client (whichever connected last) ever
        // received anything, silently starving the other.
        {
          std::lock_guard<std::mutex> lk(clients_mu_);
          clients_.insert(this);
        }
        flush_pending(this);
        // The client watchdogs the first 8s of a new connection and cancels
        // if nothing at all comes back -- normal idle operation (no trip to
        // dispatch yet) would otherwise flap the stream forever. A heartbeat
        // right on connect satisfies that check while we wait for a real
        // GatewayCommand.
        {
          vehicle_gateway::GatewayCommand hb;
          hb.mutable_heartbeat();
          enqueue(hb);
        }
        state_ = State::READING;
        stream_.Read(&incoming_, this);
        break;

      case State::READING:
        if (!ok) {
          spdlog::warn("[Stream] client disconnected");
          {
            std::lock_guard<std::mutex> lk(clients_mu_);
            clients_.erase(this);
          }
          state_ = State::FINISH;
          stream_.Finish(grpc::Status::OK, this);
          return;
        }
        handle_incoming(incoming_);
        stream_.Read(&incoming_, this);
        break;

      case State::FINISH: {
        std::lock_guard<std::mutex> lk(clients_mu_);
        clients_.erase(this);
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
    std::lock_guard<std::mutex> lk(clients_mu_);
    if (clients_.empty()) {
      spdlog::warn("[Stream] No clients connected — queuing command");
      std::lock_guard<std::mutex> lk2(pending_mu_);
      pending_commands_.push(cmd);
      return;
    }
    for (auto* h : clients_) {
      h->enqueue(cmd);
    }
  }

  static std::mutex clients_mu_;
  static std::set<VehicleStreamHandler*> clients_;
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
      // autoware_agent sends this ack exactly when its own trip state
      // machine transitions DRIVING_TO_PICKUP -> WAITING_FOR_MOVE -- i.e.
      // the vehicle has arrived at pickup and is waiting for the actual
      // trip-start command. Broadcast that as its own event so flutter-ivi
      // (which never sends TripInitAck itself) can show the Start Trip
      // button at exactly the right moment.
      if (ev.trip_init_ack().success()) {
        vehicle_gateway::GatewayCommand cmd;
        cmd.mutable_pickup_arrived();
        VehicleStreamHandler::push_command(cmd);
      }
    } else if (ev.has_trip_park_ack()) {
      spdlog::info("[Stream] TripParkAck success={}",
                   ev.trip_park_ack().success());
    } else if (ev.has_order_update_location_ack()) {
      spdlog::info("[Stream] OrderUpdateLocationAck success={}",
                   ev.order_update_location_ack().success());
    } else if (ev.has_order_update_status_ack()) {
      spdlog::info("[Stream] OrderUpdateStatusAck success={}",
                   ev.order_update_status_ack().success());
    } else if (ev.has_trip_cancel_ack()) {
      spdlog::info("[Stream] TripCancelAck success={}",
                   ev.trip_cancel_ack().success());
    }

    else if (ev.has_eta()) {
      auto& r = ev.eta();
      spdlog::info(
          "[Stream] ETA received from Autoware: vin={} reqId={} time={}",  // ADD
          r.vin_number(), r.request_id(), r.time());
      nlohmann::json j;
      j["vinNumber"] = r.vin_number();
      j["requestId"] = r.request_id();
      j["estimatedFare"] = r.fare();
      j["estimatedTime"] = r.time();
      j["status"] = r.status();
      publish_(constants::VehicleGatewayConstants::kTopicTripEta, j.dump(),
               [](bool ok, std::string e) {
                 if (!ok)
                   spdlog::error("[Stream] ETA failed: {}", e);
                 else
                   spdlog::info(
                       "[Stream] ETA published to MQTT successfully");  // ADD
               });
    } else if (ev.has_status()) {
      auto& r = ev.status();
      nlohmann::json j;
      j["vinNumber"] = r.vin_number();
      j["tripId"] = r.trip_id();
      j["tripStatus"] = r.status();
      publish_(constants::VehicleGatewayConstants::kTopicTripStatus, j.dump(),
               [](bool ok, std::string e) {
                 if (!ok) spdlog::error("[Stream] Status failed: {}", e);
               });
    } else if (ev.has_arrive()) {
      auto& r = ev.arrive();
      spdlog::info("[Stream] arrive received from Autoware");
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