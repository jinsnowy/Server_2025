#include "stdafx.h"
#include "Session.h"
#include "Core/Network/Connection.h"
#include "Core/System/Scheduler.h"
#include "Core/Network/Packet/Internal.h"
#include "Core/Network/Protocol.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"

namespace Network {
    Session::Session(const std::shared_ptr<System::Context>& context)
        :
        ActorClass(System::Channel(context))
    {
    }

    Session::Session()
        :
        ActorClass(System::Channel()),
        connection_(nullptr)
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
        connection_->Post([ip, port, session = shared_from_this()](Connection& connection) {
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
            connection->Post([](Connection& conn) {
                conn.Disconnect();
            });
            connection_ = nullptr;
        }
    }


    template<typename InternalPacket>
    static Buffer CreateBuffer(const InternalPacket& packet) {
        Buffer buffer(packet.length() + sizeof(PacketHeader));
        const PacketHeader& header = packet.header();
        BufferWriter writer(buffer);
        writer.Write(&header, sizeof(PacketHeader));
        writer.Write(packet.data(), packet.length());
        return buffer;
    }

    void Session::Send(const Network::Buffer& buffer) {
        auto connection = connection_;
        if (connection == nullptr) {
            return;
        }
        if (connection->IsSynchronized()) {
            connection->Send(buffer);
			return;
        }
        connection->Post([buffer](Connection& connection) {
            connection.Send(buffer);
        });
    }

    void Session::SendInternalMessage(const std::string& message) {
        if (message.size() > PacketHeader::kMaxSize) {
            LOG_ERROR("[SESSION] Send: message size is too large : {}", message.size());
            return;
        }

        InternalMessage packet(message);
        Buffer send_buffer = CreateBuffer(packet);
        Send(send_buffer);
    }

    void Session::InstallProtocol(std::unique_ptr<Protocol> protocol) {
        DEBUG_ASSERT(IsSynchronized());
        protocol_ = std::move(protocol);
    }

    void Session::OnDisconnected() {
    }

    void Session::OnConnected() {
    }

    void Session::OnProcessPacket(const PacketSegment& packet_segment) {
        DEBUG_ASSERT(IsSynchronized());
        const size_t packetId = packet_segment.header().id;
        if (packetId <= InternalPacketId::kCount) {
            switch (packetId) {
            case InternalPacketId::kMessage:
                OnMessage(std::string(packet_segment.body(), packet_segment.body_length()));
                break;
            default:
                LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", packetId);
                break;
            }

            return;
        }

        if (protocol_ == nullptr) {
            return;
        }

        if (protocol_->ProcessMessage(*this, packetId, packet_segment) == false) {
            LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", packetId);
            Disconnect();
        }
    }

    void Session::OnMessage(const std::string& ){
    }
}