#include "stdafx.h"
#include "Session.h"
#include "Core/Network/Connection.h"
#include "Core/System/Scheduler.h"
#include "Core/Network/Packet/Internal.h"
#include "Core/Network/Protocol.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"
#include "Core/Network/OutputStream.h"
#include "Core/Network/SendNode.h"

namespace Network {
	std::atomic<int64_t> Session::session_counter_ = 1;

    Session::Session()
        :
        Actor(System::Channel::Acquire()),
        output_stream_(std::make_unique<OutputStream>()),
		session_id_(session_counter_++)
    {
    }

    Session::Session(const System::Channel& channel)
        :
        Actor(channel),
        connection_(nullptr),
        output_stream_(std::make_unique<OutputStream>()),
		session_id_(session_counter_++)
    {
    }

    Session::~Session() = default;

    void Session::Connect(const std::string& ip, const uint16_t& port) {
        DEBUG_ASSERT(IsSynchronized());
        if (connection_ != nullptr) {
            connection_->on_disconnected_delegate().Unbind();
            connection_ = nullptr;
        }

        connection_ = std::make_shared<Connection>(System::Context::Acquire());
		connection_->on_connected_delegate().BindWeak(SharedFrom(this), &Session::OnConnectedInternal);
		connection_->on_connect_failed_delegate().BindWeak(SharedFrom(this), &Session::OnConnectFailedInternal);
        connection_->protocol_factory() = [session = SharedFrom(this)]() -> std::unique_ptr<Protocol> {
            return session->CreateProtocol();
		};
        Ctrl(*connection_).Post([ip, port, session = SharedFrom(this)](Connection& connection) {
            connection.Connect(ip, port);
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

    void Session::Send(std::unique_ptr<SendNode> node) {
        auto connection = connection_;
        if (connection == nullptr) {
            return;
		}
		connection->Send(std::move(node));
	}

    void Session::BeginSession(std::shared_ptr<Connection> connection) {
        if (connection == nullptr) {
            LOG_ERROR("[SESSION] BeginSession: connection is null");
            return;
        }

        Ctrl(*connection).Post([session = SharedFrom(this)](Connection& conn) {
            conn.on_connected_delegate().BindWeak(session, &Session::OnConnectedInternal);
            conn.protocol_factory() = [session]() -> std::unique_ptr<Protocol> {
                return session->CreateProtocol();
			};
            conn.BeginConnection();
        });
    }

    void Session::OnConnectedInternal(std::shared_ptr<Connection> connection, const IPAddress&) {
        if (connection == nullptr) {
            LOG_ERROR("[SESSION] OnConnectedInternal: connection is null");
            return;
		}

        Ctrl(*this).Post([connection](Session& session) {
            session.connection_ = connection;
            session.OnConnected(connection->connected_address());
            connection->on_disconnected_delegate().BindWeak(SharedFrom(&session), &Session::OnDisconnectedInternal);
            connection->on_data_received_delegate().BindWeak(SharedFrom(&session), &Session::OnDataReceivedInternal);
         });
    }

    void Session::OnConnectFailedInternal(const IPAddress& address, const std::string& error_message) {
        Ctrl(*this).Post([this, address, error_message](Session& session) {
            session.OnConnectFailed(address, error_message);
		});
    }

    void Session::OnDisconnectedInternal() {
        Ctrl(*this).Post([this](Session& session) {
            session.connection_ = nullptr;
            session.OnDisconnected();
        });
    }

    void Session::OnDataReceivedInternal() {
        Ctrl(*this).Post([this](Session& session) {
            session.OnProcessPacket();
		});
    }

    void Session::OnDisconnected() {
    }

    void Session::OnConnected(const IPAddress&) {
    }

    void Session::OnConnectFailed(const IPAddress&, const std::string&) {
    }

    void Session::OnProcessPacket() {
        DEBUG_ASSERT(IsSynchronized());
        
        if (connection_ == nullptr) {
            LOG_ERROR("[SESSION] OnProcessPacket: connection is null");
            return;
		}

        auto& protocol = connection_->protocol();
        if (protocol == nullptr) {
            LOG_ERROR("[SESSION] OnProcessPacket: protocol is null");
            return;
		}

        if (protocol->ProcessMessage(*this) == false) {
            Disconnect();
        }
    }

    std::string Session::GetConnectionString() const {
        auto connection = connection_;
        if (connection == nullptr) {
            return "connection none";
        }
        return connection->ToString();
	}
}