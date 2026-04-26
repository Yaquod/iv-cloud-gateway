// #ifndef VEHICLE_GATEWAY_SERVICE_H
// #define VEHICLE_GATEWAY_SERVICE_H
//
// #include <grpcpp/grpcpp.h>
// #include <spdlog/spdlog.h>
//
// #include <memory>
// #include <mutex>
// #include <thread>
//
// #include "../../build/proto/vehicle_gateway.grpc.pb.h"
// #include "transport/http_client/http_client.h"
// #include "transport/mqtt_client/mqtt_client.h"
//
// using cloud_gateway::HttpClient;
// using gateway::transport::MqttClient;
// using grpc::ServerContext;
// using grpc::Status;
//
// class VehicleGatewayServiceImp {
//  public:
//   VehicleGatewayServiceImp(MqttClient* mqtt, HttpClient* http);
//   ~VehicleGatewayServiceImp();
//
//   void Run(grpc::ServerCompletionQueue* cq);
//   void Shutdown();
//
//   vehicle_gateway::VehicleGateway::AsyncService* GetAsyncService() {
//     return &async_service_;
//   }
//
//   void SetAuthToken(const std::string& token) {
//     std::lock_guard<std::mutex> lock(token_mutex);
//     auth_token = token;
//   }
//
//   std::string GetAuthToken() {
//     std::lock_guard<std::mutex> lock(token_mutex);
//     return auth_token;
//   }
//
//  private:
//   std::mutex mutex_;
//   MqttClient* mqttClient;
//   HttpClient* httpClient;
//
//   bool shutdown_requested = false;
//   std::string auth_token;
//   std::mutex token_mutex;
//   vehicle_gateway::VehicleGateway::AsyncService async_service_;
//
//   // Base class for all async call data
//   class CallData {
//    public:
//     virtual void Proceed() = 0;
//     virtual ~CallData() = default;
//   };
//
//   class UpdateVehicleLocationCallData : public CallData {
//    public:
//     UpdateVehicleLocationCallData(
//         vehicle_gateway::VehicleGateway::AsyncService* service,
//         grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client);
//     void Proceed() override;
//
//    private:
//     vehicle_gateway::VehicleGateway::AsyncService* service_;
//     grpc::ServerCompletionQueue* cq_;
//     grpc::ServerContext ctx_;
//     MqttClient* mqtt_client_;
//
//     vehicle_gateway::UpdateVehicleLocationRequest request_;
//     vehicle_gateway::UpdateVehicleLocationResponse response_;
//     grpc::ServerAsyncResponseWriter<
//         vehicle_gateway::UpdateVehicleLocationResponse>
//         responder_;
//
//     enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
//     CallStatus status_;
//   };
//
//   class EtaCallData : public CallData {
//    public:
//     EtaCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
//                 grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client);
//     void Proceed() override;
//
//    private:
//     vehicle_gateway::VehicleGateway::AsyncService* service_;
//     grpc::ServerCompletionQueue* cq_;
//     grpc::ServerContext ctx_;
//     MqttClient* mqtt_client_;
//
//     vehicle_gateway::EtaRequest request_;
//     vehicle_gateway::EtaResponse response_;
//     grpc::ServerAsyncResponseWriter<vehicle_gateway::EtaResponse> responder_;
//
//     enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
//     CallStatus status_;
//   };
//
//   class StatusCallData : public CallData {
//    public:
//     StatusCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
//                    grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client);
//     void Proceed() override;
//
//    private:
//     vehicle_gateway::VehicleGateway::AsyncService* service_;
//     grpc::ServerCompletionQueue* cq_;
//     grpc::ServerContext ctx_;
//     MqttClient* mqtt_client_;
//
//     vehicle_gateway::StatusRequest request_;
//     vehicle_gateway::StatusResponse response_;
//     grpc::ServerAsyncResponseWriter<vehicle_gateway::StatusResponse>
//     responder_;
//
//     enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
//     CallStatus status_;
//   };
//
//   class ArriveCallData : public CallData {
//    public:
//     ArriveCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
//                    grpc::ServerCompletionQueue* cq, MqttClient* mqtt_client);
//     void Proceed() override;
//
//    private:
//     vehicle_gateway::VehicleGateway::AsyncService* service_;
//     grpc::ServerCompletionQueue* cq_;
//     grpc::ServerContext ctx_;
//     MqttClient* mqtt_client_;
//
//     vehicle_gateway::ArriveRequest request_;
//     vehicle_gateway::ArriveResponse response_;
//     grpc::ServerAsyncResponseWriter<vehicle_gateway::ArriveResponse>
//     responder_;
//
//     enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
//     CallStatus status_;
//   };
//   class TripInitCallData : public CallData {
//    public:
//     TripInitCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
//                      grpc::ServerCompletionQueue* cq, MqttClient*
//                      mqtt_client);
//     void Proceed() override;
//
//    private:
//     vehicle_gateway::VehicleGateway::AsyncService* service_;
//     grpc::ServerCompletionQueue* cq_;
//     grpc::ServerContext ctx_;
//     MqttClient* mqtt_client_;
//
//     vehicle_gateway::TripInitRequest request_;
//     vehicle_gateway::TripInitResponse response_;
//     grpc::ServerAsyncResponseWriter<vehicle_gateway::TripInitResponse>
//         responder_;
//
//     enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
//     CallStatus status_;
//   };
//
//   class TripMoveCallData : public CallData {
//    public:
//     TripMoveCallData(vehicle_gateway::VehicleGateway::AsyncService* service,
//                      grpc::ServerCompletionQueue* cq, MqttClient*
//                      mqtt_client);
//     void Proceed() override;
//
//    private:
//     vehicle_gateway::VehicleGateway::AsyncService* service_;
//     grpc::ServerCompletionQueue* cq_;
//     grpc::ServerContext ctx_;
//     MqttClient* mqtt_client_;
//
//     vehicle_gateway::TripMoveRequest request_;
//     vehicle_gateway::TripMoveResponse response_;
//     grpc::ServerAsyncResponseWriter<vehicle_gateway::TripMoveResponse>
//         responder_;
//
//     enum CallStatus { CREATE, PROCESS, WAIT_MQTT, FINISH };
//     CallStatus status_;
//   };
// };
//
// #endif