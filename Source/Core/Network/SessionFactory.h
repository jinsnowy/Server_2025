#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace Network {
    class Connection;
    class Session;
	class Protocol;
    class SessionFactory final {
    public:
        SessionFactory();
        ~SessionFactory();

        template<typename SessionClass>
        SessionFactory& SetSessionClass() {
            session_factory_ = []() {
                return std::make_shared<SessionClass>();
            };
            return *this;
        }

        SessionFactory& OnConnect(std::function<bool(std::shared_ptr<Session>)> on_connect) {
            on_connect_ = on_connect;
            return *this;
        }

        bool is_empty() const {
            return session_factory_ == nullptr;
		}

    private:
        friend class Listener;

        std::function<std::shared_ptr<Session>()> session_factory_;
        std::function<bool(std::shared_ptr<Session>)> on_connect_;

        void OnConnect(std::shared_ptr<Connection> connection);
    };
}
