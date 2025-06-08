#include "stdafx.h"
#include "Core/System/Program.h"
#include "Server/Session/ServerSession.h"
#include "Server/Session/ClientSession.h"
#include "Server/Protocol/ServerProtocol.h"
#include "Server/Protocol/ClientProtocol.h"
#include "Server/Protocol/ServerHandlerMap.h"
#include "Server/Protocol/ClientHandlerMap.h"
#include "Server/Authenticator/Authenticator.h"
#include <grpcpp/grpcpp.h>

#include "Protobuf/Private/Protos/Grpc/helloworld.grpc.pb.h"

// The service implementation
class GreeterServiceImpl final : public helloworld::Greeter::Service {
public:
    grpc::Status SayHello(grpc::ServerContext* context, const helloworld::HelloRequest* request, helloworld::HelloReply* reply) override {
        std::string prefix = "Hello, ";
        reply->set_message(prefix + request->name());
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

using namespace RTS;

std::shared_ptr<Network::Listener> listener_;
std::vector<std::shared_ptr<RTS::ServerSession>> server_sessions_;
std::vector<std::shared_ptr<RTS::ClientSession>> client_sessions_;

void ConnectMany(int32_t count) {
    auto& scheduler = System::Scheduler::Current();
    for (int32_t i = 0; i < count; ++i) {
        auto session = std::make_shared<RTS::ClientSession>();
        session->Connect("127.0.0.1", 9911);
        client_sessions_.push_back(session);
    }
}

int main() {
    System::Scheduler::Launch(32);

    ServerSession::RegisterHandler(&ServerHandlerMap::GetInstance());
    ClientSession::RegisterHandler(&ClientHandlerMap::GetInstance());

    Network::SessionFactory session_factory;
    session_factory.SetSessionClass<RTS::ServerSession>();
    session_factory.OnConnect([](std::shared_ptr<Network::Session> session) {
        server_sessions_.push_back(std::static_pointer_cast<RTS::ServerSession>(session));
        return true;
    });

    auto& scheduler = System::Scheduler::RoundRobin();
    scheduler.Post([session_factory]() {
        Authenticator::GetInstance().Initialize("http://localhost:8080");

        auto& current = System::Scheduler::Current();
        listener_ = std::make_shared<Network::Listener>(current.GetContext(), session_factory);
        listener_->Bind("0.0.0.0", 9911);
        listener_->Listen();
    });

    scheduler.Post([]() {
        ConnectMany(100);
     });

    RunServer();

    System::Program::Wait();

    return 0;
}
