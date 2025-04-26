#include "stdafx.h"
#include "Session.h"
#include "Core/Network/Connection.h"
#include "Core/System/Scheduler.h"

namespace Network {

Session::Session(const std::shared_ptr<System::Context>& context)
    :
    ActorClass(System::Channel(context)) {
}

Session::Session(std::shared_ptr<Connection> connection)
    : 
    ActorClass(connection->GetChannel()),
    connection_(connection) {
}

Session::~Session() {
    Disconnect();
}

void Session::Connect(const std::string& ip, const uint16_t& port) {
    DEBUG_ASSERT(IsSynchronized());
    if (connection_ != nullptr) {
        connection_->Disconnect();
        connection_ = nullptr;
    }

    connection_ = std::make_shared<Connection>(GetChannel().GetContext());
    connection_->Connect(ip, port, [session_weak=weak_from_this()](std::shared_ptr<Connection>) mutable {
        auto session = session_weak.lock();
        if (session == nullptr) {
            return false;
        }
        session->BeginSession();
        return true;
    });
}

bool Session::IsConnected() const {
    return connection_ != nullptr && connection_->IsConnected();
}

void Session::Disconnect() {
    if (connection_ != nullptr) {
        connection_->Post([](Connection& conn) {
            conn.Disconnect();
		});
    }
}

void Session::BeginSession() {
    DEBUG_ASSERT(IsSynchronized());
    connection_->BeginSession(shared_from_this());
    OnConnect();
}

void Session::SendMessage(const std::string& message) {
    if (connection_ == nullptr) {
        return;
    }
    connection_->Send(message);
}

void Session::OnDisconnect() {
    connection_ = nullptr;
}

void Session::OnConnect() {
}

bool Session::OnReceive(std::string message) {
    return true;
}


}