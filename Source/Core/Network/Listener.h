#pragma once

#include "Core/ThirdParty/BoostAsio.h"
#include "Core/System/Actor.h"

namespace System {
	class Context;
} // namespace System {

namespace Network {
    class Connection;
    class Acceptor;
    class Socket;
    class SessionFactory;
    class Listener : public System::Actor {
    public:
        Listener(std::shared_ptr<System::Context> context, SessionFactory session_factory);
        ~Listener();

        void Bind(const std::string& ip, const uint16_t& port);
        void Listen();
        void Stop();

        std::string ToString() const;

    private:
        bool is_listening_ = false;
        std::shared_ptr<System::Context> context_;
        std::unique_ptr<Acceptor> acceptor_;
        std::unique_ptr<SessionFactory> session_factory_;

        void Accept();
        void AcceptInternal();
        void OnAccept(std::unique_ptr<Socket> socket, const boost::system::error_code& error);
    };
}
