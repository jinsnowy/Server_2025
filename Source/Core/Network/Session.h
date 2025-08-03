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
class IPAddress;
class SendNode;
class Session : public System::Actor {
public:
    Session(const std::shared_ptr<System::Context>& context);
    Session();
    ~Session();

    void Connect(const std::string& ip, const uint16_t& port);
    bool IsConnected() const;
    void Disconnect();
    void Send(std::unique_ptr<SendNode> node);

    std::shared_ptr<Connection> connection() const { return connection_; }
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

    // Acceptor 
	void BeginSession(std::shared_ptr<Connection> connection);

    void OnProcessPacket();
    void Flush();
	void OnConnectedInternal(std::shared_ptr<Connection> connection, const IPAddress& address);
	void OnConnectFailedInternal(const IPAddress& address, const std::string& error_message);
	void OnDisconnectedInternal();
	void OnDataReceivedInternal();

protected:
    virtual void OnConnected(const IPAddress& address);
    virtual void OnConnectFailed(const IPAddress& address, const std::string& error_message);
    virtual void OnDisconnected();
};

}  
