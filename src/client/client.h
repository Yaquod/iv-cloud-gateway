    //
    // Created by alaa-hassan on 22‏/11‏/2025.
    //

    #ifndef YAQOUD_PROJECT_CLIENT_H
    #define YAQOUD_PROJECT_CLIENT_H
    #include <grpcpp/grpcpp.h>
    #include "vehicle_gateway.grpc.pb.h"
    #include <string>
    #include <spdlog/spdlog.h>

     void start_trip_flow();

    class VechileGatewayClient {

    public:

        VechileGatewayClient(const std::string &server_add);
        void Login(const std::string &vin_number_val,
                   const std::string & trip_id_val

            );

        void SendEta (
        const std::string &vin_number_val,
        const std::string & trip_id_val,
        double time_val,
        double fare_val
            );

        void SendStatus(
        const std::string &vin_number_val,
        const std::string & trip_id_val,
        const std::string & status_val
            );

        void SendArrive(
        const std::string &vin_number_val,
        const std::string & trip_id_val,
        double long_val,
        double lat_val
            );



    private:
        std::unique_ptr<vehicle_gateway::VehicleGateway::Stub>stub;


    };
    #endif //YAQOUD_PROJECT_CLIENT_H