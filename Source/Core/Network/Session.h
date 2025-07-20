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

	virtual std::unique_ptr<Protocol> CreateProtocol() = 0;

    std::string GetConnectionString() const;
	int64_t session_id() const { return session_id_; }

protected:
    std::shared_ptr<Connection> connection_;
    std::unique_ptr<OutputStream> output_stream_;

private:
    friend class SessionFactory;
    friend class Connection;

	int64_t session_id_ = 0;
	static std::atomic<int64_t> session_counter_;

    void OnProcessPacket(const std::shared_ptr<Protocol> protocol);

protected:
    virtual void OnConnected();
    virtual void OnDisconnected();
};

}  
