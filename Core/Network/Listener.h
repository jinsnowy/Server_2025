#pragma once

#include <boost/asio.hpp>

namespace Network {
    class Connection;
    class SessionFactory;
    class Listener : public std::enable_shared_from_this<Listener> {
    public:
        Listener(boost::asio::io_context& io_context, SessionFactory session_factory);
        ~Listener();

        void Bind(const std::string& ip, const uint16_t& port);
        void Start();
        void Stop();

    private:
        boost::asio::io_context& io_context_;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
        std::unique_ptr<SessionFactory> session_factory_;

        void Accept();
        void AcceptInternal();
    };
}
