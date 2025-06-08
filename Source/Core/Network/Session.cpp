#include "stdafx.h"
#include "Session.h"
#include "Core/Network/Connection.h"
#include "Core/System/Scheduler.h"
#include "Core/Network/Packet/Internal.h"
#include "Core/Network/Protocol.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"
#include "Core/Network/OutputStream.h"


namespace Network {
    Session::Session(const std::shared_ptr<System::Context>& context)
        :
        Actor(System::Channel(context)),
        output_stream_(std::make_unique<OutputStream>())
    {
    }

    Session::Session()
        :
        Actor(System::Channel()),
        connection_(nullptr),
        output_stream_(std::make_unique<OutputStream>())
    {
    }

    Session::~Session() {
        Disconnect();
    }

    void Session::Connect(const std::string& ip, const uint16_t& port) {
        DEBUG_ASSERT(IsSynchronized());
        if (connection_ != nullptr) {
            Disconnect();
        }

        connection_ = std::make_shared<Connection>(GetChannel().GetContext());
        Ctrl(*connection_).Post([ip, port, session = GetShared(this)](Connection& connection) {
            connection.Connect(ip, port, session);
        });
    }

    bool Session::IsConnected() const {
        auto connection = connection_;
        return connection != nullptr && connection->IsConnected();
    }

    void Session::Disconnect() {
        auto connection = connection_;
        if (connection != nullptr) {
            Ctrl(*connection).Post([](Connection& conn) {
                conn.Disconnect();
            });
            connection_ = nullptr;
        }
    }

    void Session::FlushToSendStream() {
        DEBUG_ASSERT(IsSynchronized());

        auto next = output_stream_->Flush();
        if (next.has_value() == false) {
            return;
        }

        auto connection = connection_;
        if (connection == nullptr) {
            return;
        }

        Ctrl(*connection).Post([buffer = next.value()](Connection& connection) {
            connection.Send(buffer);
        });
    }

    void Session::OnDisconnected() {
    }

    void Session::OnConnected() {
    }

    std::unique_ptr<Protocol> Session::CreateProtocol() {
        return std::unique_ptr<Protocol>();
    }

    void Session::OnProcessPacket(const std::shared_ptr<Protocol> protocol) {
        DEBUG_ASSERT(IsSynchronized());
        if (protocol->ProcessMessage(*this) == false) {
            Disconnect();
        }
    }
}