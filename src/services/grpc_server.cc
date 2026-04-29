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

#include "grpc_server.h"

#include "rpc_dispatcher.h"
#include "spdlog/spdlog.h"
#include "stream_tag.h"
#include "vehicle_stream_handler.h"

namespace gateway::services {

GrpcServer::GrpcServer(const std::string& listen_addr, PublishFn publish_fn)
    : listen_addr_(listen_addr), publish_fn_(publish_fn) {}

GrpcServer::~GrpcServer() { stop(); }

void GrpcServer::start() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_addr_, grpc::InsecureServerCredentials());
  builder.RegisterService(&async_service_);
  cq_ = builder.AddCompletionQueue();
  server_ = builder.BuildAndStart();

  if (!server_) {
    spdlog::error("[gRPCServer] Failed to start");
    return;
  }

  spdlog::info("[gRPCServer] Started on {}", listen_addr_);

  // Existing point-to-point handlers (keep during migration)
  new EtaCallData(&async_service_, cq_.get(), publish_fn_);
  new StatusCallData(&async_service_, cq_.get(), publish_fn_);
  new ArriveCallData(&async_service_, cq_.get(), publish_fn_);
  new UpdateLocationCallData(&async_service_, cq_.get(), publish_fn_);
  new TripInitCallData(&async_service_, cq_.get(), publish_fn_);
  new TripMoveCallData(&async_service_, cq_.get(), publish_fn_);

  // NEW — bidirectional stream handler
  new VehicleStreamHandler(&async_service_, cq_.get(), publish_fn_);

  cq_thread_ = std::thread([this]() { runCQ(); });
}

void GrpcServer::stop() {
  if (server_) {
    spdlog::info("[gRPCServer] Shutting down the server...");
    server_->Shutdown();
  }

  if (cq_) {
    cq_->Shutdown();
  }

  if (cq_thread_.joinable()) {
    cq_thread_.join();
  }
}

void GrpcServer::wait() {
  if (server_) {
    server_->Wait();
  }
}

void GrpcServer::runCQ() {
  void* tag;
  bool ok;
  while (cq_->Next(&tag, &ok)) {
    static_cast<services::IStreamTag*>(tag)->Proceed(ok);
  }
}

}  // namespace gateway::services