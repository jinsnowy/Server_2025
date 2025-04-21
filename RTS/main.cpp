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
        std::cout << "HelloSession::OnConnect" << session_id_ << std::endl;
    }

    void OnDisconnect() override {
        std::cout << "HelloSession::OnDisconnect" << session_id_ << std::endl;
    }

private:
    int session_id_;
};

std::vector<std::shared_ptr<HelloSession>> hello_sessions_;

int main() {

    System::Scheduler::Launch(4);

    Network::SessionFactory session_factory;
    session_factory.SetSessionClass<HelloSession>();
    session_factory.OnConnect([](std::shared_ptr<Network::Session> session) {
        session->Send("Hello, World!");
        hello_sessions_.push_back(std::static_pointer_cast<HelloSession>(session));
        return true;
    });

    Network::Listener listener(System::Scheduler::RoundRobin().GetIoContext(), session_factory);
    listener.Bind("0.0.0.0", 8080);
    listener.Start();

    System::Program::Wait();

    return 0;
}