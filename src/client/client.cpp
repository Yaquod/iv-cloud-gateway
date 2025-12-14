#include "../../build/proto/vehicle_gateway.pb.h"
#include "client.h"

#include <optional>
#include <boost/spirit/home/x3/core/call.hpp>
#include <boost/system/detail/mutex.hpp>
#include <nlohmann/json.hpp>

#include "../mqtt_topics.h"


VechileGatewayClient::VechileGatewayClient(const std::string &server_add) {
    auto channel = grpc::CreateChannel(server_add, grpc::InsecureChannelCredentials());
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);

    if (!channel->WaitForConnected(deadline)) {
        spdlog::error("[CLIENT]Failed to connect to server!");
    } else {
        spdlog::info("[CLIENT]Successfully connected to server");
    }

    stub = vehicle_gateway::VehicleGateway::NewStub(channel);

    auto state = channel->GetState(true);
    spdlog::info("[CLIENT]Channel state: {}", static_cast<int>(state));


    cq_thread = std::thread(&VechileGatewayClient::ProcessCompletionQueue, this);
}

VechileGatewayClient::~VechileGatewayClient()
{
    shutdown_requested = true;
    cq.Shutdown();
    if (cq_thread.joinable()) {
        cq_thread.join();
    }
}

void VechileGatewayClient::ProcessCompletionQueue() {
    void* tag;
    bool ok;

    while (cq.Next(&tag, &ok)) {
        AsyncCallDataBase* call = static_cast<AsyncCallDataBase*>(tag);

        std::lock_guard<std::mutex> lock(call->mu);

        if (ok && call->status.ok()) {
           // spdlog::info("[CLIENT]Async call succeeded");
            call->result = true;
        } else {
            if (!ok) {
                spdlog::error("[CLIENT]Async call failed - operation not ok");
            } else {
                spdlog::error("[CLIENT]Async call failed - Status: {}", call->status.error_message());
            }
            call->result = false;
        }

        call->done = true;
        call->cv.notify_one();
    }

    spdlog::info("[CLIENT]Completion queue thread exiting");
}

bool VechileGatewayClient::Login(const std::string &vin_number_val, const std::string &trip_id_val) {
   // spdlog::info("[CLIENT]Login: Creating request");

    vehicle_gateway::LoginRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);

    auto call = std::make_unique<LoginCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

   // spdlog::info("Login: Making async call with completion queue");

    call->response_reader = stub->AsyncVechileLogin(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
   // spdlog::info("Login: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
  //  spdlog::info("Login: Completed with result: {}", result);

    return result;
}

bool VechileGatewayClient::SendEta(
    const std::string &vin_number_val,
    const std::string &request_id_val,
    double time_val,
    double fare_val) {

   // spdlog::info("SendEta: Creating request");

    vehicle_gateway::EtaRequest req;
    req.set_vin_number(vin_number_val);
    req.set_request_id(request_id_val);
    req.set_time(time_val);
    req.set_fare(fare_val);

    auto call = std::make_unique<EtaCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

   // spdlog::info("SendEta: Making async call with completion queue");

    call->response_reader = stub->AsyncSendEta(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
    // spdlog::info("SendEta: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
    // spdlog::info("SendEta: Completed with result: {}", result);

    return result;
}

bool VechileGatewayClient::SendStatus(
    const std::string &vin_number_val,
    const std::string &trip_id_val,
    const std::string &status_val) {

    // spdlog::info("SendStatus: Creating request");

    vehicle_gateway::StatusRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_status(status_val);

    auto call = std::make_unique<StatusCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

    // spdlog::info("SendStatus: Making async call with completion queue");

    call->response_reader = stub->AsyncSendStatus(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
    // spdlog::info("SendStatus: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
    // spdlog::info("SendStatus: Completed with result: {}", result);

    return result;
}

bool VechileGatewayClient::SendArrive(
    const std::string &vin_number_val,
    const std::string &trip_id_val,
    double long_val,
    double lat_val) {

    // spdlog::info("SendArrive: Creating request");

    vehicle_gateway::ArriveRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_long_(long_val);
    req.set_lat(lat_val);

    auto call = std::make_unique<ArriveCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

    // spdlog::info("SendArrive: Making async call with completion queue");

    call->response_reader = stub->AsyncSendArrive(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
    // spdlog::info("SendArrive: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
   // spdlog::info("SendArrive: Completed with result: {}", result);

    return result;
}


//this section will be part of client class later

std::mutex mqtt_mut;
std::condition_variable mqtt_cv;
std::deque<std::pair<std::string , std::string>> mqtt_queue;
std::thread trip_thread;
TripData trip_data;
std::atomic<bool> trip_running{false};
std::atomic<bool> status_running{false};
std::thread status_thread;

void on_mqtt_message(const std::string& topic , const std::string payload)
{
 std::lock_guard<std::mutex> lock(mqtt_mut);
    mqtt_queue.push_back({topic , payload});
    mqtt_cv.notify_one();
}


bool WaitForMqttMessage(const std::string expected_topic)
{
    // I will add timeout later
  std::unique_lock<std::mutex> lock(mqtt_mut);
    mqtt_cv.wait(lock , [&] {

        if (!trip_running) {

            return true;

        }

       for (auto &msg :mqtt_queue) {
           if (msg.first == expected_topic) return true;
       }
        return false;
    });

    if (!trip_running) {
        return false ;
    }

    std::string payload;
    for (auto it = mqtt_queue.begin(); it != mqtt_queue.end(); ++it) {
        if (it->first == expected_topic) {
            payload = it->second;
            mqtt_queue.erase(it);
            break;
        }
    }

    parsing_received_message(expected_topic, payload);
    return true;
}




void parsing_received_message(const std::string& topic, const std::string& payload)
{
    try {
        if (topic == MqttTopics::TripInit)
        {
            spdlog::info("[TRIP]received message: {}", topic);

            auto json = nlohmann::json::parse(payload);

            auto vin = json["vin_number"].get<std::string>();
            auto trip = json["request_id"].get<std::string>();
            auto slong = json["starting_long"].get<double>();
            auto slat = json["starting_lat"].get<double>();
            auto elong = json["ending_long"].get<double>();
            auto elat = json["ending_lat"].get<double>();

            spdlog::info("[TRIP] {} , {} , {} , {} , {} , {}", vin, trip, slong, slat, elong, elat);

            trip_data.trip_id = trip;
            trip_data.start_long = slong;
            trip_data.start_lat = slat;
            trip_data.end_long = elong;
            trip_data.end_lat = elat;
        }
        else if (topic == MqttTopics::TripMove) {
            spdlog::info("[TRIP]received message: {}, topic", payload);

            auto json = nlohmann::json::parse(payload);

            auto vin = json["vin_number"].get<std::string>();
            auto trip = json["trip_id"].get<std::string>();
            auto lon = json["long"].get<double>();
            auto lat = json["lat"].get<double>();

            spdlog::info("[TRIP] {} , {} , {} , {}", vin, trip, lon, lat);
        }
        else {
            spdlog::info("[TRIP]undefined topic");
        }
    }
    catch (const std::exception& e) {
        spdlog::error("[TRIP]Exception in parsing message: {}", e.what());
    }
}

bool IsMovement() {
    return trip_data.trip_status == TripState::MOVE_TO_DEST ||
           trip_data.trip_status == TripState::MOVE_TO_PARK ||
           trip_data.trip_status == TripState::MOVE_TO_PICKUP ;
}


void StatusSending(VechileGatewayClient &client) {
    while (trip_running && status_running) {
        if (IsMovement()) {
            //get current location


            bool status_success = client.SendStatus(trip_data.vin_number , trip_data.trip_id , trip_data.vechile_status);
            if (status_success) {
                spdlog::info("[TRIP]status send successfully");
            }
            else {
                spdlog::error("[TRIP]failed to send status");
            }


        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
}


void start_trip_flow(const std::string &vin_number , const std::string  &trip_id)
{

    trip_data.trip_id = trip_id;
    trip_data.vin_number = vin_number;
    trip_data.trip_status = TripState::IDLE;
    trip_running = true;
    trip_thread = std::thread(&TripFlow);
}


void StopTripFlow() {


    trip_running = false;
    status_running = false;

    mqtt_cv.notify_all();

    if (status_thread.joinable()) {
        status_thread.join();
    }


    if (trip_thread.joinable()) {
        trip_thread.join();
    }

}


void SetTripState(TripState state) {
   trip_data.trip_status = state;
}


void TripFlow() {
    VechileGatewayClient client("localhost:50051");

    status_running = true;
    status_thread = std::thread(StatusSending , std::ref(client));

  while (trip_running)
      {
      try
      {
          switch (trip_data.trip_status) {


              case TripState::IDLE: {
               SetTripState(TripState::LOGIN);
                  break;
              }
              case TripState::LOGIN: {

                //  bool login_success = client.Login(trip_data.vin_number , trip_data.trip_id);
                  //remove it later
                  bool login_success = true;
                  if (login_success) {
                      spdlog::info("[TRIP]login successfully.");
                      SetTripState(TripState::WAIT_INIT);
                  }
                  else {
                      spdlog::info("[TRIP]login failed");
                  }


                  break;

              }
                  case TripState::WAIT_INIT: {

                  spdlog::info("[TRIP]car waiting for init....");

                  if (!WaitForMqttMessage(MqttTopics::TripInit))
                  {
                      spdlog::error("[TRIP]failed to receive init");
                      break;

                  }
                  spdlog::info("[TRIP]trip INIT is received. ");
                  trip_data.trip_status = TripState::PATH_PLANNING_FOR_WHOLE_TRIP;
                  break;
                  }


              case TripState::PATH_PLANNING_FOR_WHOLE_TRIP:
                  {
                  spdlog::info("[TRIP]calling autoware for whole trip path planning....");

                  //calling autoware

                  std::this_thread::sleep_for(std::chrono::seconds(path_planning_time));

                  trip_data.time = 600;
                  trip_data.fare = 60.0;
                  SetTripState(TripState::SEND_ETA);
                  break;


              }
              case TripState::SEND_ETA:
                  {
                  bool eta_success = client.SendEta(trip_data.vin_number , trip_data.trip_id , trip_data.time , trip_data.fare);
                  if (!eta_success) {
                      spdlog::error("[TRIP]failed to send eta.");
                      break;
                  }

                  spdlog::info("[TRIP]eta send to cloud.");
                  trip_data.trip_status = TripState::WAIT_MOVE_TO_PICKUP;
                  break;
              }
                  case TripState::WAIT_MOVE_TO_PICKUP: {
                  spdlog::info("[TRIP]car waiting to move to pick up...");

                  if (!WaitForMqttMessage(MqttTopics::TripMove))
                  {
                      spdlog::error("[TRIP]failed to receive move.");
                      break;

                  }
                  spdlog::info("[TRIP]trip move is received.");

                  trip_data.trip_status = TripState:: PATH_PLANNING_TO_PICKUP;
                  break;

              }
                  case TripState:: PATH_PLANNING_TO_PICKUP: {

                  spdlog::info("[TRIP]calling autoware for pickup path planning...");
                  std::this_thread::sleep_for(std::chrono::seconds(path_planning_time));

                  SetTripState(TripState::MOVE_TO_PICKUP);
                  break;
              }

                  case TripState::MOVE_TO_PICKUP: {
                  spdlog::info("[TRIP]car is now moving to pickup location...");

                  std::this_thread::sleep_for(std::chrono::seconds(moving_time));
                  SetTripState(TripState::ARRIVE_TO_PICKUP);


                  break;
              }
                  case TripState::ARRIVE_TO_PICKUP: {
                  bool arrive_sucess = client.SendArrive(trip_data.vin_number , trip_data.trip_id , trip_data.lon , trip_data.lat);
                  if (!arrive_sucess) {
                      spdlog::error("[TRIP]failed to send arrive to pickup.");
                      break;
                  }

                  spdlog::info("[TRIP]arrive to pickup is  send to cloud.");
                  trip_data.trip_status = TripState::WAIT_MOVE_TO_DEST;

                  break;
              }

              case TripState::WAIT_MOVE_TO_DEST : {

                  spdlog::info("[TRIP]car waiting for move to dest...");

                  if (!WaitForMqttMessage(MqttTopics::TripMove))
                  {
                      spdlog::error("[TRIP]failed to receive move to dest.");
                      break;

                  }
                  spdlog::info("[TRIP]move to dest is received. ");
                  trip_data.trip_status = TripState::PATH_PLANNING_TO_DEST;
                  break;
              }

                  case TripState::PATH_PLANNING_TO_DEST: {
                  spdlog::info("[TRIP]calling autoware for dest path planning...");
                  std::this_thread::sleep_for(std::chrono::seconds(path_planning_time));

                  SetTripState(TripState::MOVE_TO_DEST);
                  break;

              }
                  case TripState::MOVE_TO_DEST: {
                  spdlog::info("[TRIP]car is now moving to DEST location...");

                  std::this_thread::sleep_for(std::chrono::seconds(moving_time));
                  SetTripState(TripState::ARRIVE_TO_DEST);
                  break;


              }

              case TripState::ARRIVE_TO_DEST: {
                  bool arrive_sucess = client.SendArrive(trip_data.vin_number , trip_data.trip_id , trip_data.lon , trip_data.lat);
                  if (!arrive_sucess) {
                      spdlog::error("[TRIP]failed to send arrive to dest.");
                      break;
                  }

                  spdlog::info("[TRIP]arrive to dest is  send to cloud.");
                  SetTripState(TripState::WAIT_MOVE_TO_PARK);

                  break;
              }

                  case TripState::WAIT_MOVE_TO_PARK: {
                  spdlog::info("[TRIP]car waiting for move to park...");

                  if (!WaitForMqttMessage(MqttTopics::TripMove))
                  {
                      spdlog::error("[TRIP]failed to receive move to park.");
                      break;

                  }
                  spdlog::info("[TRIP]move to park is received.");
                  SetTripState(TripState::PATH_PLANNING_TO_PARK);
                  break;
              }

                  case TripState::PATH_PLANNING_TO_PARK: {
                  spdlog::info("[TRIP]calling autoware for park path planning...");
                  std::this_thread::sleep_for(std::chrono::seconds(path_planning_time));

                  SetTripState(TripState::MOVE_TO_PARK);
                  break;

              }

                  case TripState::MOVE_TO_PARK: {
                  spdlog::info("[TRIP]car is now moving to PARK location...");

                  std::this_thread::sleep_for(std::chrono::seconds(moving_time));
                  SetTripState(TripState::ARRIVE_TO_PARK);
                  break;

              }

                  case TripState::ARRIVE_TO_PARK: {
                  bool arrive_sucess = client.SendArrive(trip_data.vin_number , trip_data.trip_id , trip_data.lon , trip_data.lat);
                  if (!arrive_sucess) {
                      spdlog::error("[TRIP]failed to send arrive to park.");
                      break;
                  }

                  spdlog::info("[TRIP]arrive to park is  send to cloud.");
                 SetTripState(TripState::FINISHED);

                  break;

              }
                  case TripState::FINISHED: {
                  spdlog::info("[TRIP]trip is finished.");
                  trip_running = false;
                  status_running = false;

              }
          }

      }
      catch (std::exception &e) {
          spdlog::error("[TRIP]Exception in trip flow: {}", e.what());
          trip_running = false;
          status_running = false;

      }
  }

}