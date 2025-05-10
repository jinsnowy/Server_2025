#include "stdafx.h"
#include "Listener.h"
#include "Core/Network/SessionFactory.h"
#include "Core/System/Scheduler.h"
#include "Core/Network/Connection.h"
#include "Core/Network/Acceptor.h"
#include "Core/Network/Socket.h"

namespace Network {
    Listener::Listener(std::shared_ptr<System::Context> context, SessionFactory session_factory)
        :
        context_(context),
        acceptor_(std::make_unique<Acceptor>(context)),
        session_factory_(std::make_unique<SessionFactory>(std::move(session_factory))) {
    }

    Listener::~Listener() {
        Stop();
    }

    void Listener::Bind(const std::string& ip, const uint16_t& port) {
        acceptor_->Bind(ip, port);
    }

    void Listener::Listen() {
        DEBUG_ASSERT(IsSynchronized());
        is_listening_ = true;
        acceptor_->Listen();
        Accept();
    }

    void Listener::Stop() {
        DEBUG_ASSERT(IsSynchronized());
        is_listening_ = false;
        acceptor_->Stop();
    }

    std::string Listener::ToString() const {
        return acceptor_->ToString();
    }

    void Listener::Accept() {
        DEBUG_ASSERT(IsSynchronized());
        AcceptInternal();
    }

    void Listener::AcceptInternal() {
        acceptor_->AcceptAsync(&Listener::OnAccept, shared_from_this());
    }

    void Listener::OnAccept(std::unique_ptr<Socket> socket, const boost::system::error_code& error) {
        DEBUG_ASSERT(IsSynchronized());
        if (is_listening_ == false) {
            return;
        }

        if (error == boost::asio::error::operation_aborted) {
            AcceptInternal();
            return;
        }

        if (error) {
            LOG_ERROR("Listener::AcceptInternal error: {}, error_code:{}", error.message(), error.value());
            AcceptInternal();
            return;
        }

        auto connection = std::make_shared<Connection>(std::move(socket));
        session_factory_->OnConnect(connection);

        AcceptInternal();
    }
}