#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace Network {
    class Connection;
    class Session;
    class SessionFactory final {
    public:
        SessionFactory();
        ~SessionFactory();

        template<typename SessionClass>
        SessionFactory& SetSessionClass() {
            session_factory_ = [](std::shared_ptr<Connection> conn) {
                return std::make_shared<SessionClass>(conn);
            };
            return *this;
        }

        SessionFactory& OnConnect(std::function<bool(std::shared_ptr<Session>)> on_connect) {
            on_connect_ = on_connect;
            return *this;
        }

    private:
        friend class Listener;

        std::function<std::shared_ptr<Session>(std::shared_ptr<Connection>)> session_factory_;
        std::function<bool(std::shared_ptr<Session>)> on_connect_;

        void OnConnect(std::shared_ptr<Connection> connection);
    };
}
