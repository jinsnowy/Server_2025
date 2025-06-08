#include "stdafx.h"
#include "Protocol.h"
#include "Core/Network/Packet/Internal.h"
#include "Core/Misc/ApplyVariants.h"

namespace Network
{
    bool Protocol::ProcessReceiveData( const PacketSegment& segment)
    {
        const size_t packetId = segment.header().id;
        if (packetId <= static_cast<size_t>(InternalPacketId::kCount)) {
            switch (static_cast<InternalPacketId>(packetId)) {
            case InternalPacketId::kMessage:
                internal_messages_.Enqueue(InternalMessage{
                    .message = std::string(segment.body(), segment.body_length())
                });
                break;
            default:
                LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", packetId);
                break;
            }

            return true;
        }

        //

        return false;
    }

    bool Protocol::ProcessMessage(Session&)
    {
        for (auto message = internal_messages_.Dequeue(); message.has_value(); message = internal_messages_.Dequeue()) {
            Misc::Apply(
                message.value(),
                [](const InternalMessage& msg) {
                    LOG_INFO("[SESSION] Received message: {}", msg.message);
                },
                [](const InternalPing& ping) {
                    LOG_INFO("[SESSION] Received ping: id={}, time={}", ping.ping_id, ping.send_time.ToString());
                },
                [](const InternalPong& pong) {
                    LOG_INFO("[SESSION] Received pong: id={}, time={}", pong.ping_id, pong.send_time.ToString());
                }
            );
        }

        return true;
    }

}

