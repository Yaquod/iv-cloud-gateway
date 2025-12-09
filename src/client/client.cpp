#include "../../build/proto/vehicle_gateway.pb.h"
#include "client.h"

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
            spdlog::info("Async call succeeded");
            call->result = true;
        } else {
            if (!ok) {
                spdlog::error("Async call failed - operation not ok");
            } else {
                spdlog::error("Async call failed - Status: {}", call->status.error_message());
            }
            call->result = false;
        }

        call->done = true;
        call->cv.notify_one();
    }

    spdlog::info("Completion queue thread exiting");
}

bool VechileGatewayClient::Login(const std::string &vin_number_val, const std::string &trip_id_val) {
    spdlog::info("Login: Creating request");

    vehicle_gateway::LoginRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);

    auto call = std::make_unique<LoginCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

    spdlog::info("Login: Making async call with completion queue");

    call->response_reader = stub->AsyncVechileLogin(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
    spdlog::info("Login: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
    spdlog::info("Login: Completed with result: {}", result);

    return result;
}

bool VechileGatewayClient::SendEta(
    const std::string &vin_number_val,
    const std::string &trip_id_val,
    double time_val,
    double fare_val) {

    spdlog::info("SendEta: Creating request");

    vehicle_gateway::EtaRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_time(time_val);
    req.set_fare(fare_val);

    auto call = std::make_unique<EtaCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

    spdlog::info("SendEta: Making async call with completion queue");

    call->response_reader = stub->AsyncSendEta(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
    spdlog::info("SendEta: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
    spdlog::info("SendEta: Completed with result: {}", result);

    return result;
}

bool VechileGatewayClient::SendStatus(
    const std::string &vin_number_val,
    const std::string &trip_id_val,
    const std::string &status_val) {

    spdlog::info("SendStatus: Creating request");

    vehicle_gateway::StatusRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_status(status_val);

    auto call = std::make_unique<StatusCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

    spdlog::info("SendStatus: Making async call with completion queue");

    call->response_reader = stub->AsyncSendStatus(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
    spdlog::info("SendStatus: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
    spdlog::info("SendStatus: Completed with result: {}", result);

    return result;
}

bool VechileGatewayClient::SendArrive(
    const std::string &vin_number_val,
    const std::string &trip_id_val,
    double long_val,
    double lat_val) {

    spdlog::info("SendArrive: Creating request");

    vehicle_gateway::ArriveRequest req;
    req.set_vin_number(vin_number_val);
    req.set_trip_id(trip_id_val);
    req.set_long_(long_val);
    req.set_lat(lat_val);

    auto call = std::make_unique<ArriveCallData>();
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

    spdlog::info("SendArrive: Making async call with completion queue");

    call->response_reader = stub->AsyncSendArrive(&call->ctx, req, &cq);
    call->response_reader->Finish(&call->response, &call->status, (void*)call.get());

    std::unique_lock<std::mutex> lock(call->mu);
    spdlog::info("SendArrive: Waiting for completion");
    call->cv.wait(lock, [&call] { return call->done; });

    bool result = call->result;
    spdlog::info("SendArrive: Completed with result: {}", result);

    return result;
}

void start_trip_flow()
{

}
