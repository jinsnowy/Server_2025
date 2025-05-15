#pragma once

#include "Core/System/Actor.h"

namespace Network {
    class Buffer;
} // namespace Network

namespace System {
	class Context;
} // namespace System

namespace Network {
struct PacketSegment;
class Protocol;
class Connection;
class Buffer;
class Session : public System::Actor<Session> {
public:
    Session(const std::shared_ptr<System::Context>& context);
    Session();
    ~Session();

    void Connect(const std::string& ip, const uint16_t& port);
    bool IsConnected() const;
    void Disconnect();
    
    void Send(const Network::Buffer& buffer);
    void SendInternalMessage(const std::string& message);
    
    std::shared_ptr<Connection> connection() const { return connection_; }
    void set_connection(std::shared_ptr<Connection> connection) { connection_ = connection; }

protected:
    std::shared_ptr<Connection> connection_;

    void InstallProtocol(std::unique_ptr<Protocol> protocol);

private:
    friend class SessionFactory;
    friend class Connection;

    std::unique_ptr<Protocol> protocol_;

    void OnProcessPacket(const PacketSegment& packet_segment);

protected:
    virtual void OnConnected();
    virtual void OnDisconnected();

    virtual void OnMessage(const std::string& message);
};

}  
