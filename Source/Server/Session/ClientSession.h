#pragma once

#include "Protobuf/Public/ProtobufSession.h"

namespace RTS {
    class ClientHandlerMap;
    class ClientSession : public Protobuf::ProtobufSession {
    public:
        ClientSession();

        void OnConnected() override ;

        void OnDisconnected() override {
            LOG_INFO("ClientSession::OnDisconnected session_id:{}, address:{}", session_id_, connection()->ToString());
        }

        std::unique_ptr<Network::Protocol> CreateProtocol() override;

        int32_t session_id() const { return session_id_; }

        static void RegisterHandler(ClientHandlerMap* handler_map);

    private:
        int session_id_ = 0;
        static int session_id_counter_;
    };
}