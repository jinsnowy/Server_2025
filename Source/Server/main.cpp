#include "stdafx.h"
#include "Core/Logging/Logger.h"
#include "Core/System/Program.h"
#include "Core/Json/Json.h"
#include "Server/Session/LobbySession.h"
#include "Server/Session/ClientSession.h"
#include "Server/Session/WorldSession.h"
#include "Server/Service/LobbyServiceDef.h"
#include "Server/Service/ClientServiceDef.h"
#include "Server/Service/WorldServiceDef.h"
#include "Server/Service/ServiceBuilder.h"
#include "Server/Service/LobbyService.h"
#include "Server/DataTable/SpawnerDataRecord.h"
#include "Core/Sql/Database.h"
#include "Server/Authenticator/Authenticator.h"
#include "InterServer/GrpcService.h"
#include "InterServer/LobbyGrpcService.h"
#include "Service/WorldService.h"
#include "Protobuf/Public/User.h"
#include "Database/DB.h"
#include "Protobuf/Public/ProtoUtils.h"

// The service implementation

using namespace Server;

std::shared_ptr<Service> lobby_server;
std::shared_ptr<WorldService> world_server;
std::vector<std::shared_ptr<Server::LobbySession>> lobby_sessions;
std::vector<std::shared_ptr<Server::WorldSession>> world_sessions;
std::vector<std::shared_ptr<Server::ClientSession>> client_sessions_;

void ConnectMany(int32_t count) {
    //auto& scheduler = System::Scheduler::Current();
    for (int32_t i = 0; i < count; ++i) {
        auto session = std::make_shared<Server::ClientSession>();
        session->Connect("127.0.0.1", 9911);
        client_sessions_.push_back(session);
    }
}

void TimerTest() {
    srand((uint32_t)time(NULL));
    for (int64_t i = 0; i < 1000; ++i) {
        System::Scheduler::ForEach([](System::Scheduler& scheduler) {
            int32_t random_value = rand() % 30000 + 1; // Random value between 1 and 1000
            auto expected = System::Time::Now() + System::Duration::FromMilliseconds(random_value);
            scheduler.context().timer_context().Reserve(random_value, [expected]() {
                auto current_time = System::Time::Now();
                auto diff = current_time - expected;
                LOG_INFO("After Timer Current Time : {}, diff: {}ms", current_time.ToString(), diff.Milliseconds());
          });
        });
    }
}

void PeriodicTimerTest() {
    LOG_INFO("Starting Periodic Timer Test : {}", System::Time::Now().ToString());
    System::PeriodicTimer::Schedule(System::Duration::FromSeconds(5), [](System::PeriodicTimer::Handle&) {
        auto now = System::Time::Now();
        LOG_INFO("Current Time: {}", now.ToString());
    }, true);
}

DBConfig GetDBConfig() {
    DBConfig config;
    config.lobby_db_dsn = L"Driver={ODBC Driver 17 for SQL Server};Server=localhost,1433;UID=sa;PWD=StrongPass1!;Database=LobbyDB;";
    config.game_db_dsn = L"Driver={ODBC Driver 17 for SQL Server};Server=localhost,1433;UID=sa;PWD=StrongPass1!;Database=GameDB;";
    return config;
}

int main() {
	DataTable::SpawnerDataRecord::Load("./Data/SpawnAreaActor.json");

    System::Scheduler::CreateThreadPool(4);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::EnableDefaultHealthCheckService(true);
	DB::GetInstance().Initialize(GetDBConfig());

    LobbySession::RegisterHandler(&LobbyHandlerMap::GetInstance());
    ClientSession::RegisterHandler(&ClientHandlerMap::GetInstance());
	WorldSession::RegisterHandler(&WorldHandlerMap::GetInstance());

    auto& scheduler = System::Scheduler::RoundRobin();
    scheduler.Post([]() {
        Authenticator::GetInstance().Initialize("http://localhost:8080");

		ServiceBuilder lobby_service_builder;
        lobby_server = lobby_service_builder.SessionClass<Server::LobbySession>()
            .OnConnect([](std::shared_ptr<Network::Session> session) {
                lobby_sessions.push_back(std::static_pointer_cast<Server::LobbySession>(session));
                return true;
             })
            .UsePort(9911)
            .Build<LobbyService>();
        lobby_server->Start();

		LOG_INFO("Lobby service started on port 9911");

        ServiceBuilder world_service_builder;
        world_server = world_service_builder.SessionClass<Server::WorldSession>()
            .OnConnect([](std::shared_ptr<Network::Session> session) {
                world_sessions.push_back(std::static_pointer_cast<Server::WorldSession>(session));
                return true;
            })
            .UsePort(9912)
            .Build<WorldService>();
        world_server->set_lobby_server_address(LobbyGrpcService::kConnectAddress);
        world_server->Start();

        PeriodicTimerTest();

        LOG_INFO("World service started on port 9912");
    });


    System::Program::Wait();

    System::Scheduler::Destroy();
	Log::Logger::Destroy();
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
