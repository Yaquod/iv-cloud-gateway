//
// Created by alaa-hassan on 19‏/11‏/2025.
//
#include "vehicle_gateway_service.h"

#include <boost/mqtt5/mqtt_client.hpp>


VehicleGatewayServiceImp::VehicleGatewayServiceImp(MqttClient* mqtt,HttpClient* http)
 : mqttClient(mqtt),httpClient(http){

}

VehicleGatewayServiceImp::~VehicleGatewayServiceImp() = default;

 grpc::ServerUnaryReactor* VechileLogin(grpc::CallbackServerContext* context, const vehicle_gateway::LoginRequest* request, vehicle_gateway::LoginRespose* response)
{

  auto *reactor = context->DefaultReactor();


 std::string url = "/api/vechile/login";
 std::string payload = request->DebugString();

 auto httpResponse = httpClient->Post(url, {}, payload);
 if (!httpResponse.success) {
  response->set_success(false);
  response->set_message("HTTP login failed: " + httpResponse.error_message);
  reactor->Finish(grpc::StatusCode::INTERNAL ,"Http login failed");
  return reactor;
 }

 response->set_success(true);
 response->set_message("login ok");
  reactor->Finish(grpc::Status::ok);
 return reactor;
}

    grpc::ServerUnaryReactor* SendEta(grpc::CallbackServerContext* context, const vehicle_gateway::EtaRequest* request, vehicle_gateway::EtaResponse* response)
 {
      auto *reactor = context->DefaultReactor();


      std::string topic = "topic/trip/eta";
      mqttClient->mqtt_publish(topic , request->DebugString() ,
       [reactor , response](bool success , std::string message ) {
        if (!success) {
         response->set_success(false);
         response->set_message("mqtt publish Eta failed: " + message);
         reactor->Finish(grpc::StatusCode::INTERNAL ,"Mqtt publish Eta failed");
         return ;
        }
        response->set_success(true);
        response ->set_message("successfully published Eta");
        reactor->Finish(grpc::StatusCode::OK);

       }
       );

  return reactor;
}


grpc::ServerUnaryReactor* SendStatus(grpc::CallbackServerContext* context, const vehicle_gateway::StatusRequest* request, vehicle_gateway::StatusResponse* response)
{

  auto * reactor = context->DefaultReactor();
 std::string topic = "topic/trip/status";
 mqttClient->mqtt_publish(topic , request->DebugString() ,
 [reactor , response](bool success , std::string message ) {
  if (!success) {

   response->set_success(false);
   response->set_message("mqtt publish Status failed: " + message);
   reactor->Finish(grpc::StatusCode::INTERNAL ,"Mqtt publish Status failed");
   return ;
  }

       response->set_success(true);
       response ->set_message("successfully published status");
       reactor->Finish(grpc::StatusCode::OK);
 }
 );
  return reactor;

}



grpc::ServerUnaryReactor* SendArrive(grpc::CallbackServerContext* context, const vehicle_gateway::ArriveRequest* request, vehicle_gateway::ArriveResponse* response)
 {
  auto * reactor = context->DefaultReactor();
 std::string topic = "topic/trip/arrive";
  mqttClient->mqtt_publish(topic , request->DebugString() ,
  [reactor , response](bool success , std::string message ) {
   if (!success) {
    response->set_success(false);
    response->set_message("mqtt publish Arrive failed: " + message);
    reactor->Finish(grpc::StatusCode::INTERNAL,"Mqtt publish Arrive failed");
    return ;
   }
  }
  );

  response->set_success(true);
  response->set_message("successfully published Arrive");
  reactor->Finish(grpc::StatusCode::OK);

  return reactor;
}





