#include "stdafx.h"
#include "Session.h"
#include "Core/Network/Connection.h"
#include "Core/System/Scheduler.h"

namespace Network {

Session::Session(std::shared_ptr<Connection> connection)
    : connection_(connection) {
}

Session::~Session() {
    Disconnect();
}

void Session::Connect(const std::string& ip, const uint16_t& port) {
    if (connection_ != nullptr) {
        connection_->Disconnect();
        connection_ = nullptr;
    }

    auto connection = std::make_shared<Connection>(System::Scheduler::Current().GetIoContext());
    connection->Connect(ip, port, [session_weak=weak_from_this()](std::shared_ptr<Connection> conn)mutable {
        auto session = session_weak.lock();
        if (session == nullptr) {
            return false;
        }
        session->SetConnection(conn);
        return true;
    });
}

bool Session::IsConnected() const {
    return connection_ != nullptr && connection_->IsConnected();
}

void Session::Disconnect() {
    if (connection_ != nullptr) {
        connection_->Disconnect();
    }
}

void Session::SetConnection(std::shared_ptr<Connection> connection) {
    connection_ = connection;
    connection_->SetSession(shared_from_this());
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