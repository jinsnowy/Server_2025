#include "stdafx.h"
#include "Core/System/Program.h"
#include "RTS/Session/ServerSession.h"
#include "RTS/Session/ClientSession.h"
#include "RTS/Protocol/ServerProtocol.h"
#include "RTS/Protocol/ClientProtocol.h"
#include "RTS/Protocol/ServerHandlerMap.h"
#include "RTS/Protocol/ClientHandlerMap.h"

using namespace RTS;

std::shared_ptr<Network::Listener> listener_;
std::vector<std::shared_ptr<RTS::ServerSession>> server_sessions_;
std::vector<std::shared_ptr<RTS::ClientSession>> client_sessions_;

void ConnectMany(int32_t count) {
    auto& scheduler = System::Scheduler::Current();
    for (int32_t i = 0; i < count; ++i) {
        auto session = std::make_shared<RTS::ClientSession>();
        session->Connect("127.0.0.1", 8080);
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
        auto& current = System::Scheduler::Current();
        listener_ = std::make_shared<Network::Listener>(current.GetContext(), session_factory);
        listener_->Bind("0.0.0.0", 8080);
        listener_->Listen();
    });

    scheduler.Post([]() {
        ConnectMany(1);
    });

    System::Program::Wait();

    return 0;
}
