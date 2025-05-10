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
        ActorClass(System::Channel(context)),
        send_stream_(std::make_unique<SendNetworkStream>()),
        recv_stream_(std::make_unique<RecvNetworkStream>())
    {
    }

    Session::Session(std::shared_ptr<Connection> connection)
        :
        ActorClass(connection->GetChannel()),
        connection_(connection),
        send_stream_(std::make_unique<SendNetworkStream>()),
        recv_stream_(std::make_unique<RecvNetworkStream>())
    {
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
            connection_ = nullptr;
        }
    }

    void Session::SendMessage(const std::string& message) {
        DEBUG_ASSERT(IsSynchronized());
        if (connection_ == nullptr) {
            return;
        }
        if (message.size() > PacketHeader::kMaxSize) {
            LOG_ERROR("[SESSION] Send: message size is too large : {}", message.size());
            return;
        }

        InternalMessage packet(message);

        const auto header = packet.header();
        auto outputStream = GetOutputStream();
        if (outputStream.WriteAliasedRaw(&header, sizeof(PacketHeader)) == false) {
            return;
        }
        if (outputStream.WriteAliasedRaw(packet.data(), packet.length()) == false) {
            return;
        }

        FlushSend();
    }

    OutputStream Session::GetOutputStream() {
        return OutputStream(*send_stream_);
    }

    void Session::FlushSend(bool continueOnWriter) {
        DEBUG_ASSERT(IsSynchronized());
        if (connection_ == nullptr) {
            return;
        }

        if (continueOnWriter == false && send_stream_->is_sending == true) {
            return;
        }

        if (send_stream_->buffers.empty() || send_stream_->buffers.front().IsEmpty()) {
            send_stream_->is_sending = false;
            return;
        }

        auto& buffer = send_stream_->buffers.front();
        if (buffer.GetByteCount() < 0) {
            LOG_ERROR("[SESSION] FlushSend: invalid buffer size : {}", buffer.GetByteCount());
            Disconnect();
            return;
        }

        send_stream_->is_sending = true;
        connection_->Send(buffer);

        buffer.set_start_pos(buffer.end_pos());
        if (buffer.GetRemainingByteCount() <= PacketHeader::Size()) {
			send_stream_->buffers.pop_front();
		}
    }

    void Session::InstallProtocol(std::unique_ptr<Protocol> protocol) {
        protocol_ = std::move(protocol);
    }

    void Session::BeginSession() {
        DEBUG_ASSERT(IsSynchronized());
        if (connection_ == nullptr) {
            return;
        }
        connection_->BeginSession(shared_from_this());
        
        send_stream_->is_sending = false;
        recv_stream_->buffer.Allocate(Buffer::kDefault);

        BeginReceive();
        OnConnected();
    }

    void Session::BeginReceive() {
        connection_->BeginReceive(recv_stream_->buffer.buffer_shared(), Buffer::kDefault);
    }

    void Session::OnDisconnected() {
    }

    void Session::OnConnected() {
    }

    bool Session::OnReceived(size_t length) {
        DEBUG_ASSERT(IsSynchronized());

        if (length < PacketHeader::Size()) {
            LOG_ERROR("[SESSION] OnReceived: invalid length is too small : {}", length);
            return false;
        }

        const char* data = recv_stream_->buffer.data();
        uint32_t dataLength = static_cast<uint32_t>(length);
        uint32_t readOffset = 0;
        while (dataLength > 0) {
            const char* readDataPtr = data + readOffset;

            if (recv_stream_->pending.has_value()) {
                auto& pendingStream = recv_stream_->pending.value();
                uint32_t currentReadableSize = std::min(pendingStream.remainSegmentLength, dataLength);
                uint32_t currentPacketBufferOffset = pendingStream.packetBuffer.size() - pendingStream.remainSegmentLength;
                memcpy_s(pendingStream.packetBuffer.data() + currentPacketBufferOffset, pendingStream.remainSegmentLength, readDataPtr, currentReadableSize);
                pendingStream.remainSegmentLength -= currentReadableSize;
                if (pendingStream.remainSegmentLength == 0) {
                    if (OnProcessPacket(PacketSegment{ pendingStream.packetBuffer.data(), static_cast<uint32_t>(pendingStream.packetBuffer.size()) }) == false) {
                        return false;
                    }
                    recv_stream_->pending.reset();
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

                    recv_stream_->pending.emplace(PendingStream{
                        .header = *header,
                        .packetBuffer = std::move(packetBuffer),
                        .remainSegmentLength = packetReadableSize - dataLength,
                    });

                    return true;
                }

                if (OnProcessPacket(PacketSegment{ readDataPtr, packetReadableSize }) == false) {
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

        if (protocol_ == nullptr) {
            return false;
        }

        return protocol_->ProcessMessage(*this, packetId, packet_segment);
    }

    void Session::OnMessage(const std::string& ){
    }
}