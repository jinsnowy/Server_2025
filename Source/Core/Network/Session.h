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
class BufferView;
class OutputStream;
class Session : public System::Actor {
public:
    Session(const std::shared_ptr<System::Context>& context);
    Session();
    ~Session();

    void Connect(const std::string& ip, const uint16_t& port);
    bool IsConnected() const;
    void Disconnect();

    std::shared_ptr<Connection> connection() const { return connection_; }
    void set_connection(std::shared_ptr<Connection> connection) { connection_ = connection; }
    void FlushToSendStream();

protected:
    std::shared_ptr<Connection> connection_;
    std::unique_ptr<OutputStream> output_stream_;

    virtual std::unique_ptr<Protocol> CreateProtocol() = 0;

private:
    friend class SessionFactory;
    friend class Connection;

    void OnProcessPacket(const std::shared_ptr<Protocol> protocol);

protected:
    virtual void OnConnected();
    virtual void OnDisconnected();
};

}  
