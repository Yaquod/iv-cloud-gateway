    #ifndef YAQOUD_PROJECT_CLIENT_H
    #define YAQOUD_PROJECT_CLIENT_H
    #include <grpcpp/grpcpp.h>
    #include "vehicle_gateway.grpc.pb.h"
    #include <string>
    #include <spdlog/spdlog.h>
    #include <thread>
    #include <condition_variable>
    #include <atomic>
    #include <deque>



#define path_planning_time 5
#define moving_time 15


enum class TripState {
    IDLE,
    LOGIN,
    WAIT_INIT,
    PATH_PLANNING_FOR_WHOLE_TRIP,
    SEND_ETA,
    WAIT_MOVE_TO_PICKUP,
    PATH_PLANNING_TO_PICKUP,
    MOVE_TO_PICKUP,
    ARRIVE_TO_PICKUP,
    WAIT_MOVE_TO_DEST,
    PATH_PLANNING_TO_DEST,
    MOVE_TO_DEST,
    ARRIVE_TO_DEST,
    WAIT_MOVE_TO_PARK,
    PATH_PLANNING_TO_PARK,
    MOVE_TO_PARK,
    ARRIVE_TO_PARK,
    FINISHED
};


  struct TripData
{
      std::string trip_id;
      std::string vin_number;
      TripState trip_status = TripState::IDLE;
      std::string vechile_status ;
      double start_long;
      double start_lat;
      double end_long;
      double end_lat;
      double fare;
      double time;
      double lon;
      double lat;



  };



    void start_trip_flow(const std::string &vin_number , const std::string  &trip_id);
    void StopTripFlow();
    void SetTripState(TripState state);
    void TripFlow();
    void on_mqtt_message(const std::string& topic , const std::string payload);
    bool WaitForMqttMessage(const std::string topic);
    void  parsing_received_message(const std::string& topic, const std::string& payload) ;
    bool IsMovement();









    class VechileGatewayClient
    {
    public:
        VechileGatewayClient(const std::string &server_add);
        ~VechileGatewayClient();

        bool Login(const std::string &vin_number_val, const std::string &trip_id_val);
        bool SendEta(const std::string &vin_number_val, const std::string &request_id_val, double time_val, double fare_val);
        bool SendStatus(const std::string &vin_number_val, const std::string &trip_id_val, const std::string &status_val);
        bool SendArrive(const std::string &vin_number_val, const std::string &trip_id_val, double long_val, double lat_val);

    private:
        std::unique_ptr<vehicle_gateway::VehicleGateway::Stub> stub;
        grpc::CompletionQueue cq;
        std::thread cq_thread;
        std::atomic<bool> shutdown_requested{false};

        void ProcessCompletionQueue();


        struct AsyncCallDataBase
        {
            grpc::ClientContext ctx;
            grpc::Status status;
            bool done = false;
            bool result = false;
            std::mutex mu;
            std::condition_variable cv;
            virtual ~AsyncCallDataBase() = default;
        };


        struct LoginCallData : AsyncCallDataBase
        {
            vehicle_gateway::LoginRespose response;
            std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::LoginRespose>> response_reader;
        };

        struct EtaCallData : AsyncCallDataBase {
            vehicle_gateway::EtaResponse response;
            std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::EtaResponse>> response_reader;
        };

        struct StatusCallData : AsyncCallDataBase {
            vehicle_gateway::StatusResponse response;
            std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::StatusResponse>> response_reader;
        };

        struct ArriveCallData : AsyncCallDataBase {
            vehicle_gateway::ArriveResponse response;
            std::unique_ptr<grpc::ClientAsyncResponseReader<vehicle_gateway::ArriveResponse>> response_reader;
        };
    };


     void StatusSending(VechileGatewayClient & client);

    #endif //YAQOUD_PROJECT_CLIENT_H


