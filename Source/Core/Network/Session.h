#pragma once

#include "Core/System/Actor.h"
#include "Core/Network/PendingStream.h"

namespace System {
	class Context;
} // namespace System

namespace Network {
struct PacketSegment;
struct PendingRecvStream;

class Connection;
class Session : public System::Actor<Session> {
public:
    Session(const std::shared_ptr<System::Context>& context);
    Session(std::shared_ptr<Connection> connection);
    ~Session();

    void Connect(const std::string& ip, const uint16_t& port);
    bool IsConnected() const;
    void Disconnect();
    void SendMessage(const std::string& message);
    std::shared_ptr<Connection> connection() const { return connection_; }

protected:
    std::shared_ptr<Connection> connection_;

private:
    friend class SessionFactory;
    friend class Connection;

    std::optional<PendingRecvStream> pending_recv_stream_;

    void BeginSession();
    bool OnReceived(const char* data, size_t length);

    bool OnProcessPacket(const PacketSegment& packet_segment);

    virtual void OnConnected();
    virtual void OnDisconnected();

protected: // internal packet handler;
    virtual void OnMessage(const std::string& message) {}
};

}  
