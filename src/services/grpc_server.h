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

#ifndef VEHICLECLOUDGATEWAY_GRPC_SERVER_H
#define VEHICLECLOUDGATEWAY_GRPC_SERVER_H

#include <grpcpp/server.h>

#include <functional>
#include <string>

#include "transport/http_client/http_client.h"
#include "vehicle_gateway.grpc.pb.h"

namespace gateway::services {
class GrpcServer {
 public:
  using PublishFn =
      std::function<void(const std::string& topic, const std::string& payload,
                         std::function<void(bool, std::string)> cb)>;

  explicit GrpcServer(const std::string& listen_addr, PublishFn publish_fn);
  ~GrpcServer();

  void start();

  void wait();

  void stop();

 private:
  void runCQ();

  std::string listen_addr_;
  PublishFn publish_fn_;
  std::unique_ptr<grpc::Server> server_;
  std::unique_ptr<grpc::ServerCompletionQueue> cq_;
  std::thread cq_thread_;
  vehicle_gateway::VehicleGateway::AsyncService async_service_;
};
}  // namespace gateway::services
#endif  // VEHICLECLOUDGATEWAY_GRPC_SERVER_H
