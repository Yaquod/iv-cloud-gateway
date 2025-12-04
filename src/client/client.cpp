    //
    // Created by alaa-hassan on 22‏/11‏/2025.
    //

    #include "../../build/proto/vehicle_gateway.pb.h"

    #include "client.h"

    #include <condition_variable>
#include <grpcpp/impl/codegen/client_callback.h>

    VechileGatewayClient::VechileGatewayClient(const std::string &server_add) {
        auto channel = grpc::CreateChannel(server_add, grpc::InsecureChannelCredentials());
        auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);

        if (!channel->WaitForConnected(deadline)) {
            spdlog::error("Failed to connect to server!");
        } else {
            spdlog::info("Successfully connected to server");
        }

        stub = vehicle_gateway::VehicleGateway::NewStub(channel);

        auto state = channel->GetState(true);
        spdlog::info("Channel state: {}", static_cast<int>(state));

    }

    bool VechileGatewayClient::Login(const std::string &vin_number_val,
                const std::string & trip_id_val

         )
    {
         vehicle_gateway::LoginRequest req;
        req.set_vin_number(vin_number_val);
        req.set_trip_id(trip_id_val);

        vehicle_gateway::LoginRespose resp;
         grpc::ClientContext ctx;

        bool done = false, res = false;

        std::condition_variable cv;
        std::mutex mu;

        stub->experimental_async()->VechileLogin(&ctx, &req, &resp,
        [&mu , &cv , &done ,&resp , &res](grpc::Status status) {
            bool ret;
            if (!status.ok()) {
                spdlog::info("login failed ");
                ret = false;
            }
            else {
                spdlog::info("login succeeded");
                ret = true;
            }
            res = ret;

            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        }
        );

        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        return res ;

    }

    bool VechileGatewayClient::SendEta (
        const std::string &vin_number_val,
        const std::string & trip_id_val,
        double time_val,
        double fare_val
            ) {


        spdlog::info("send eta: Creating request");

        vehicle_gateway::EtaRequest req;
        req.set_vin_number(vin_number_val);
        req.set_trip_id(trip_id_val);
        req.set_time(time_val);
        req.set_fare(fare_val);

        vehicle_gateway::EtaResponse resp;
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));


       bool done = false, res = false;

        std::condition_variable cv;
        std::mutex mu;
        spdlog::info("send eta : Acquiring lock");


        spdlog::info("send eta : Making async call");
        stub->experimental_async()->SendEta(&ctx, &req, &resp,
        [ &mu , &res , &cv,&resp ,&done](grpc::Status status) {
          bool ret;
            if (!status.ok()) {
                spdlog::info("send Eta failed ");
                spdlog::info("error status:{}",status.error_message());
               ret = false;
            }
            else {
                spdlog::info("send Eta succeeded");
               ret = true;
            }
            res = ret;

             std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        }
        );

        std::unique_lock<std::mutex> lock(mu);

        spdlog::info("send eta : Waiting for callback");
        cv.wait(lock, [&done] { return done; });



        return res ;

    }




    bool VechileGatewayClient::SendStatus(
     const std::string &vin_number_val,
     const std::string & trip_id_val,
     const std::string & status_val
         ) {
        vehicle_gateway::StatusRequest req;
        req.set_vin_number(vin_number_val);
        req.set_trip_id(trip_id_val);
        req.set_status(status_val);

        vehicle_gateway::StatusResponse resp;
        grpc::ClientContext ctx;

        bool done = false, res = false;

        std::condition_variable cv;
        std::mutex mu;

        stub->experimental_async()->SendStatus(&ctx, &req, &resp,
        [&mu , &cv , &done ,&resp , &res](grpc::Status status) {
            bool ret;
            if (!status.ok()) {
                spdlog::info("send status failed ");
                ret = false;
            }
            else {
                spdlog::info("send status succeeded");
                ret = true;
            }
            res = ret;

            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        }
        );

        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        return res ;




    }

    bool VechileGatewayClient::SendArrive(
     const std::string &vin_number_val,
     const std::string & trip_id_val,
     double long_val,
     double lat_val
         ) {
        vehicle_gateway::ArriveRequest req;
        req.set_vin_number(vin_number_val);
        req.set_trip_id(trip_id_val);
        req.set_long_(long_val);
        req.set_lat(lat_val);

        vehicle_gateway::ArriveResponse resp;
        grpc::ClientContext ctx;

        bool done = false, res = false ;

        std::condition_variable cv;
        std::mutex mu;

        stub->experimental_async()->SendArrive(&ctx, &req, &resp,
        [&mu , &cv , &done ,&resp , &res](grpc::Status status) {
            bool ret;
            if (!status.ok()) {
                spdlog::info("send arrive failed ");
                ret = false;
            }
            else {
                spdlog::info("send arrive succeeded");
                ret = true;
            }
            res = ret;

            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        }
        );

        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        return res ;


    }

    void start_trip_flow() {




    }







