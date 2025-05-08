#pragma once

#include "Core/System/Actor.h"
#include "Core/Network/OutputStream.h"

namespace System {
	class Context;
} // namespace System

namespace Network {
struct PacketSegment;
struct RecvNetworkStream;
struct SendNetworkStream;
class Protocol;
class Connection;
class Buffer;
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
    void InstallProtocol(std::unique_ptr<Protocol> protocol);

    OutputStream GetOutputStream() { return OutputStream(*send_stream_); }
    void FlushSend(bool continueOnWriter = false);

private:
    friend class SessionFactory;
    friend class Connection;

    std::unique_ptr<SendNetworkStream> send_stream_;
    std::unique_ptr<RecvNetworkStream> recv_stream_;
    std::unique_ptr<Protocol> protocol_;

    void BeginSession();
    void BeginReceive();
    bool OnReceived(size_t length);
    bool OnProcessPacket(const PacketSegment& packet_segment);

    virtual void OnConnected();
    virtual void OnDisconnected();

protected: // internal packet handler;
    virtual void OnMessage(const std::string& message);
};

}  
