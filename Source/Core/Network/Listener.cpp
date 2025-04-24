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

        LOG_INFO("Listener::Bind ip: {}, port: {}", ip, port);
    }

    void Listener::Start() {
        Accept();
        acceptor_->listen();
        LOG_INFO("Listener::Start");
    }

    void Listener::Stop() {
        acceptor_->close();
    }

    void Listener::Accept() {
        AcceptInternal();
    }

    void Listener::AcceptInternal() {
        System::Scheduler::Get(io_context_).Post([listener = shared_from_this()]() {
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(System::Scheduler::RoundRobin().GetIoContext());
            listener->acceptor_->async_accept(*socket, [socket, listener](const boost::system::error_code& error) mutable {
                listener->OnAccept(std::move(*socket), error);
            });
        });
    }

    void Listener::OnAccept(boost::asio::ip::tcp::socket&& socket, const boost::system::error_code& error) {
        if (error == boost::asio::error::operation_aborted) {
            return;
        }

        LOG_INFO("Current ThreadId {}, {}", System::Scheduler::ThreadId(),
            System::Scheduler::Get(static_cast<const boost::asio::io_context&>(socket.get_executor().context())).thread_id());

        if (error) {
            LOG_ERROR("Listener::AcceptInternal error: {}, error_code:{}", error.message(), error.value());
            return;
        }

        auto connection = std::make_shared<Connection>(std::move(socket));
        session_factory_->OnConnect(connection);

        AcceptInternal();
    }
}