#include "stdafx.h"
#include "Listener.h"
#include "Core/Network/SessionFactory.h"
#include "Core/System/Scheduler.h"
#include "Core/Network/Connection.h"

namespace Network {
    Listener::Listener(boost::asio::io_context& io_context, SessionFactory session_factory)
        :
        io_context_(io_context),
        acceptor_(std::make_unique<boost::asio::ip::tcp::acceptor>(io_context)),
        session_factory_(std::make_unique<SessionFactory>(std::move(session_factory))) {
    }

    Listener::~Listener() {
        Stop();
    }

    void Listener::Bind(const std::string& ip, const uint16_t& port) {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
        acceptor_->open(endpoint.protocol());
        acceptor_->bind(endpoint);
    }

    void Listener::Start() {
        Accept();
        acceptor_->listen();
    }

    void Listener::Stop() {
        acceptor_->close();
    }

    void Listener::Accept() {
        for (int accept_count = 0; accept_count < 64; accept_count++) {
            AcceptInternal();
        }
    }

    void Listener::AcceptInternal() {
        acceptor_->async_accept(System::Scheduler::RoundRobin().GetIoContext(), [weak_listener = weak_from_this()](const boost::system::error_code& error, boost::asio::ip::tcp::socket socket) {
            if (error == boost::asio::error::operation_aborted) {
                return;
            }

            auto listener = weak_listener.lock();
            if (listener == nullptr) {
                return;
            }

            auto& current_io_context = static_cast<boost::asio::io_context&>(socket.get_executor().context());
            auto connection = std::make_shared<Connection>(current_io_context, std::move(socket));
            listener->session_factory_->OnConnect(connection);
            listener->AcceptInternal();
        });
    }
}