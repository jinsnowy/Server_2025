#include "stdafx.h"
#include "SessionFactory.h"
#include "Core/Network/Socket.h"
#include "Core/Network/Session.h"
#include "Core/Network/Connection.h"
#include "Core/Network/Protocol.h"

namespace Network {
    SessionFactory::SessionFactory()
        : 
        session_factory_(nullptr) {
    }

    SessionFactory::~SessionFactory() {
    }

    void SessionFactory::OnConnect(std::shared_ptr<Connection> connection) {
        if (!connection->IsConnected()) {
            LOG_ERROR("SessionFactory::OnConnect connection is not connected {}", connection->ToString());
            return;
        }

        auto session = session_factory_();
        if (on_connect_) {
            bool is_session_okay = on_connect_(session);
            if (is_session_okay == false) {
                return;
            }
        }
        
        Ctrl(*connection).Post([session](Connection& connection) {
			connection.set_session(session);
            connection.OnConnected(boost::system::error_code{}, connection.socket()->raw_socket().remote_endpoint());
        });
    }
}
