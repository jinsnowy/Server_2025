#include "stdafx.h"
#include "Core/Logging/Logger.h"
#include "Core/System/Program.h"
#include "Server/Session/ServerSession.h"
#include "Server/Session/ClientSession.h"
#include "Server/Protocol/ServerProtocol.h"
#include "Server/Protocol/ClientProtocol.h"
#include "Server/Protocol/ServerHandlerMap.h"
#include "Server/Protocol/ClientHandlerMap.h"
#include "Core/Sql/Database.h"
#include "Server/Authenticator/Authenticator.h"
#include "InterServer/GrpcService.h"
#include "InterServer/HelloWorldGreeterService.h"
#include "Protobuf/Public/User.h"
#include "Database/DB.h"
// The service implementation


using namespace Server;

std::shared_ptr<Network::Listener> listener_;
std::vector<std::shared_ptr<Server::ServerSession>> server_sessions_;
std::vector<std::shared_ptr<Server::ClientSession>> client_sessions_;

void ConnectMany(int32_t count) {
    //auto& scheduler = System::Scheduler::Current();
    for (int32_t i = 0; i < count; ++i) {
        auto session = std::make_shared<Server::ClientSession>();
        session->Connect("127.0.0.1", 9911);
        client_sessions_.push_back(session);
    }
}

DBConfig GetDBConfig() {
    DBConfig config;
    config.lobby_db_dsn = L"Driver={ODBC Driver 17 for SQL Server};Server=localhost,1433;UID=dev;PWD=strongpass1!;Database=LobbyDB;";
    config.game_db_dsn = L"Driver={ODBC Driver 17 for SQL Server};Server=localhost,1433;UID=dev;PWD=strongpass1!;Database=GameDB;";
    return config;
}

int main() {
    System::Scheduler::CreateThreadPool(32);
  
	DB::GetInstance().Initialize(GetDBConfig());

    LOBBYDB.GetAgent();

    ServerSession::RegisterHandler(&ServerHandlerMap::GetInstance());
    ClientSession::RegisterHandler(&ClientHandlerMap::GetInstance());

    Network::SessionFactory session_factory;
    session_factory.SetSessionClass<Server::ServerSession>();
    session_factory.OnConnect([](std::shared_ptr<Network::Session> session) {
        server_sessions_.push_back(std::static_pointer_cast<Server::ServerSession>(session));
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

    Server::RunGrpcService<Server::HelloWorldGreeterService>("0.0.0.0:51001");

    System::Program::Wait();

    Server::StopGrpcService();
    System::Scheduler::Destroy();
	Log::Logger::Destroy();
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
