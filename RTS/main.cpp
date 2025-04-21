#include "stdafx.h"
#include "Core/System/Program.h"

int session_id_counter_ = 0;

class HelloSession : public Network::Session {
public:
    HelloSession(std::shared_ptr<Network::Connection> connection)
        :
        Network::Session(connection) {
        session_id_ = session_id_counter_++;
    }

    void OnConnect() override {
        LOG_INFO("HelloSession::OnConnect session_id:{}, address:{}", session_id_, connection()->ToString());
    }

    void OnDisconnect() override {
        LOG_INFO("HelloSession::OnDisconnect session_id:{}, address:{}", session_id_, connection()->ToString());
    }

private:
    int session_id_;
};

std::vector<std::shared_ptr<HelloSession>> hello_sessions_;
std::vector<std::shared_ptr<Network::Connection>> hello_connections_;

void ConnectMany(int32_t count) {
    auto& scheduler = System::Scheduler::Current();
    for (int32_t i = 0; i < count; ++i) {
        auto connection = std::make_shared<Network::Connection>(scheduler.GetIoContext());
        connection->Connect("127.0.0.1", 8080, [](std::shared_ptr<Network::Connection> connection) {
            return true;
        });
        hello_connections_.push_back(connection);
    }
}

int main() {
    System::Scheduler::Launch(4);

    Network::SessionFactory session_factory;
    session_factory.SetSessionClass<HelloSession>();
    session_factory.OnConnect([](std::shared_ptr<Network::Session> session) {
        session->Send("Hello, World!");
        hello_sessions_.push_back(std::static_pointer_cast<HelloSession>(session));
        return true;
    });

    auto& scheduler = System::Scheduler::RoundRobin();
    auto listener = std::make_shared<Network::Listener>(scheduler.GetIoContext(), session_factory);
    scheduler.Post([listener]() {
        listener->Bind("0.0.0.0", 8080);
        listener->Start();
    });
    scheduler.Post([]() {
        ConnectMany(100);
    });

    System::Program::Wait();

    return 0;
}