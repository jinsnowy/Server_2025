#pragma once

namespace boost {
    namespace asio {
        class io_context;
    }
}

namespace Network {

class Connection;
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(std::shared_ptr<Connection> connection);
    ~Session();

    void Connect(const std::string& ip, const uint16_t& port);
    bool IsConnected() const;
    void Disconnect();
    void Send(const std::string& message);

    std::shared_ptr<Connection> connection() const { return connection_; }

protected:
    std::shared_ptr<Connection> connection_;

private:
    friend class SessionFactory;
    friend class Connection;
    boost::asio::io_context& io_context_;

    void SetConnection(std::shared_ptr<Connection> connection);

    virtual void OnConnect();
    virtual void OnDisconnect();
    virtual bool OnReceive(std::string message);
};

}  
