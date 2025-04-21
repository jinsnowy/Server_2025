#include "stdafx.h"
#include "SessionFactory.h"
#include "Core/Network/Session.h"
#include "Core/Network/Connection.h"

namespace Network {
    SessionFactory::SessionFactory()
        : 
        session_factory_(nullptr) {
    }

    SessionFactory::~SessionFactory() {
    }

    void SessionFactory::OnConnect(std::shared_ptr<Connection> connection) {
        if (!connection->IsConnected()) {
            return;
        }

        auto session = session_factory_(connection);
        session->SetConnection(connection);

        if (on_connect_) {
            on_connect_(session);
        }
    }
}
