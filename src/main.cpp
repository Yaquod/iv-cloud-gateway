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

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <csignal>

#include "application/trip_orchestrator.h"
#include "infra/config.h"
#include "infra/constants.h"
#include "services/auth_services.h"
#include "services/grpc_server.h"
#include "services/mqtt_router.h"
#include "services/vehicle_stream_handler.h"
#include "transport/http_client/http_client.h"
#include "transport/mqtt_client/mqtt_client.h"

static std::atomic<bool> g_shutdown{false};
static gateway::services::GrpcServer* g_grpc_ptr = nullptr;

static void signal_handler(int) {
  spdlog::info("[Gateway] shutdown signal");
  g_shutdown = true;
  if (g_grpc_ptr) g_grpc_ptr->stop();
}

int main() {
  spdlog::set_default_logger(spdlog::stdout_color_mt("gateway"));
  spdlog::set_level(spdlog::level::info);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

  // TODO: replace hardcoded values with YAML / env-var loader
  gateway::Config cfg;
  cfg.vin_number = "ORIN_NANO_001";
  cfg.mqtt_broker = "localhost";
  cfg.mqtt_port = 1883;
  cfg.mqtt_client_id = "vehicle_gateway";
  cfg.grpc_listen = "0.0.0.0:50051";
  cfg.base_url = "http://localhost:8000";
  cfg.admin_email = "admin@example.com";
  cfg.admin_password = "password";
  cfg.admin_first_name = "Admin";
  cfg.admin_last_name = "User";
  cfg.admin_phone = "+1234567890";
  cfg.verify_code = "123456";
  cfg.plate_no = "ABC-1234";
  cfg.color = "white";
  cfg.car_company = "Toyota";
  cfg.model = "Prius";
  cfg.seat_no = 4;

  gateway::transport::MqttClient mqtt(cfg.mqtt_broker, cfg.mqtt_port,
                                      cfg.mqtt_client_id);

  gateway::application::TripOrchestrator orchestrator(cfg.vin_number);

  orchestrator.set_callbacks(
      {.on_trip_init =
           [](int64_t req_id, double s_lat, double s_lon, double e_lat,
              double e_lon) {
             spdlog::info("[Gateway] Pushing TripInit reqId={} to stream",
                          req_id);

             vehicle_gateway::GatewayCommand cmd;
             auto* init = cmd.mutable_trip_init();
             init->set_request_id(req_id);
             init->set_start_lat(s_lat);
             init->set_start_long(s_lon);
             init->set_end_lat(e_lat);
             init->set_end_long(e_lon);

             gateway::services::VehicleStreamHandler::push_command(cmd);
           },

       .on_trip_move =
           [](int64_t trip_id, double lat, double lon) {
             spdlog::info("[Gateway] Pushing TripMove tripId={} to stream",
                          trip_id);

             vehicle_gateway::GatewayCommand cmd;
             auto* move = cmd.mutable_trip_move();
             move->set_trip_id(trip_id);
             move->set_latitude(lat);
             move->set_longitude(lon);

             gateway::services::VehicleStreamHandler::push_command(cmd);
           }});
  gateway::services::MqttRouter router;

  router.on(gateway::constants::VehicleGatewayConstants::kTopicTripInit,
            [&orchestrator](const std::string& payload) {
              orchestrator.handle_trip_init(payload);
            });

  router.on(gateway::constants::VehicleGatewayConstants::kTopicTripMove,
            [&orchestrator](const std::string& payload) {
              orchestrator.handle_trip_move(payload);
            });

  mqtt.set_message_handler(
      [&router](const std::string& topic, const std::string& payload) {
        router.dispatch(topic, payload);
      });

  spdlog::info("[DEBUG] calling subscribe BEFORE start");
  mqtt.subscribe(gateway::constants::VehicleGatewayConstants::kTopicTripInit);
  mqtt.subscribe(gateway::constants::VehicleGatewayConstants::kTopicTripMove);

  spdlog::info("[DEBUG] calling start NOW");
  mqtt.start();

  gateway::transport::HttpClient http;
  gateway::services::AuthService auth(http, cfg);

  if (!auth.setup()) {
    spdlog::warn("[Gateway] running without authentication");
  }
  auth.create_vehicle();

  gateway::services::GrpcServer grpc_server(
      cfg.grpc_listen,
      [&mqtt](const std::string& topic, const std::string& payload,
              std::function<void(bool, std::string)> cb) {
        mqtt.publish(topic, payload, std::move(cb));
      });

  g_grpc_ptr = &grpc_server;
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  grpc_server.start();
  spdlog::info("[Gateway] ready — gRPC {} | MQTT {}:{}", cfg.grpc_listen,
               cfg.mqtt_broker, cfg.mqtt_port);

  grpc_server.wait();

  mqtt.stop();
  spdlog::info("[Gateway] clean exit");
  return 0;
}