#include "stdafx.h"
#include "Session.h"
#include "Core/Network/Connection.h"
#include "Core/System/Scheduler.h"
#include "Core/Network/Packet/Internal.h"

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
        connection_->Connect(ip, port, [session_weak = weak_from_this()](std::shared_ptr<Connection>) mutable {
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

    void Session::SendMessage(const std::string& message) {
        if (connection_ == nullptr) {
            return;
        }
        if (message.size() > PacketHeader::kMaxSize) {
            LOG_ERROR("[SESSION] Send: message size is too large : {}", message.size());
            return;
        }

        std::vector<char> buffer(PacketHeader::Size() + message.size());
        PacketHeader header = {
            .id = InternalPacketId::kMessage,
            .size = static_cast<uint32_t>(message.size())
        };
        memcpy_s(buffer.data(), PacketHeader::Size(), &header, PacketHeader::Size());
        memcpy_s(buffer.data() + PacketHeader::Size(), message.size(), message.data(), message.size());

        connection_->Send(std::move(buffer));
    }

    void Session::BeginSession() {
        DEBUG_ASSERT(IsSynchronized());
        connection_->BeginSession(shared_from_this());
        OnConnected();
    }

    void Session::OnDisconnected() {
        connection_ = nullptr;
    }

    void Session::OnConnected() {
    }

    bool Session::OnReceived(const char* data, size_t length) {
        DEBUG_ASSERT(IsSynchronized());

        if (length <= PacketHeader::Size()) {
            LOG_ERROR("[SESSION] OnReceived: invalid length is too small : {}", length);
            return false;
        }

        uint32_t dataLength = static_cast<uint32_t>(length);
        uint32_t readOffset = 0;
        while (dataLength > 0) {
            const char* readDataPtr = data + readOffset;

            if (pending_recv_stream_.has_value()) {
                auto& pendingStream = pending_recv_stream_.value();
                uint32_t currentReadableSize = std::min(pendingStream.remainSegmentLength, dataLength);
                uint32_t currentPacketBufferOffset = pendingStream.packetBuffer.size() - pendingStream.remainSegmentLength;
                memcpy_s(pendingStream.packetBuffer.data() + currentPacketBufferOffset, pendingStream.remainSegmentLength, readDataPtr, currentReadableSize);
                pendingStream.remainSegmentLength -= currentReadableSize;
                if (pendingStream.remainSegmentLength == 0) {
                    if (OnProcessPacket(PacketSegment{ pendingStream.packetBuffer.data(), static_cast<uint32_t>(pendingStream.packetBuffer.size()) }) == false) {
                        return false;
                    }
                    pending_recv_stream_.reset();
                }

                dataLength -= currentReadableSize;
                readOffset += currentReadableSize;
            }
            else {
                const PacketHeader* header = PacketHeader::Peek(readDataPtr);
                if (header->size >= PacketHeader::kMaxSize) {
                    LOG_ERROR("[SESSION] OnReceived: invalid length is too large : {}", header->size);
                    return false;
                }

                uint32_t packetReadableSize = PacketHeader::Size() + header->size;
                if (packetReadableSize > dataLength) {
                    std::vector<char> packetBuffer(packetReadableSize);
                    memcpy_s(packetBuffer.data(), packetReadableSize, readDataPtr, dataLength);

                    pending_recv_stream_.emplace(PendingRecvStream{
                        .header = *header,
                        .packetBuffer = std::move(packetBuffer),
                        .remainSegmentLength = packetReadableSize - dataLength,
                    });

                    return true;
                }

                if (OnProcessPacket(PacketSegment{ data, packetReadableSize }) == false) {
                    return false;
                }

                dataLength -= packetReadableSize;
                readOffset += packetReadableSize;
            }
        }

        return true;
    }

    bool Session::OnProcessPacket(const PacketSegment& packet_segment) {
        const size_t packetId = packet_segment.header().id;
        if (packetId <= InternalPacketId::kCount) {
            switch (packetId) {
            case InternalPacketId::kMessage:
                OnMessage(std::string(packet_segment.body(), packet_segment.body_length()));
                break;
            default:
                LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", packetId);
                return false;
            }

            return true;
        }

        return true;
    }
}