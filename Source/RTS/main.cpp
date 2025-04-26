#include "stdafx.h"
#include "Core/System/Program.h"
#include "RTS/Session/ServerSession.h"

int session_id_counter_ = 1;
std::atomic<int> session_id_counter_client_ = 1;

class HelloSession : public RTS::ServerSession {
public:
    HelloSession(std::shared_ptr<Network::Connection> conn)
        :
        RTS::ServerSession(conn) {
        session_id_ = session_id_counter_++;
    }

    void OnConnected() override {
        LOG_INFO("HelloSession::OnConnect session_id:{}, address:{}", session_id_, connection()->ToString());
    }

    void OnDisconnected() override {
        LOG_INFO("HelloSession::OnDisconnect session_id:{}, address:{}", session_id_, connection()->ToString());
    }

private:
    int session_id_;
};

class ClientSession : public Network::Session {
public:
    ClientSession(std::shared_ptr<Network::Connection> conn)
        :
        Network::Session(std::move(conn)) {
    }

    void OnConnected() override {
        session_id_ = session_id_counter_client_++;

        LOG_INFO("ClientSession::OnConnect sesion_id:{}", session_id_);

        SendMessage("Hello, server!");
    }

    void OnDisconnected() override {
        LOG_INFO("ClientSession::OnDisconnect");
    }

    void OnMessage(const std::string& message) override {
		LOG_INFO("ClientSession::OnMessage: {}", message.c_str());
	}

private:
    int session_id_;
};

std::shared_ptr<Network::Listener> listener_;
std::vector<std::shared_ptr<HelloSession>> hello_sessions_;
std::vector<std::shared_ptr<ClientSession>> client_sessions_;

void ConnectMany(int32_t count) {
    auto& scheduler = System::Scheduler::Current();
    for (int32_t i = 0; i < count; ++i) {
        auto connection = std::make_shared<Network::Connection>(scheduler.GetContext());
        auto session = std::make_shared<ClientSession>(connection);
        session->Connect("127.0.0.1", 8080);
        client_sessions_.push_back(session);
    }
}

int main() {
    System::Scheduler::Launch(32);

    Network::SessionFactory session_factory;
    session_factory.SetSessionClass<HelloSession>();
    session_factory.OnConnect([](std::shared_ptr<Network::Session> session) {
        hello_sessions_.push_back(std::static_pointer_cast<HelloSession>(session));
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
        ConnectMany(100);
    });

    System::Program::Wait();

    return 0;
}