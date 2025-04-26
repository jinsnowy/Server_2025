#pragma once

#include "Core/System/Actor.h"

namespace System {
	class Context;
} // namespace System

namespace Network {

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

    void BeginSession();

    virtual void OnConnect();
    virtual void OnDisconnect();
    virtual bool OnReceive(std::string message);
};

}  
