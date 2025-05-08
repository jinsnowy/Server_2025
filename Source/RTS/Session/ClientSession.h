#pragma once

namespace RTS {
    class ClientSession : public Network::Session {
    public:
        ClientSession(std::shared_ptr<Network::Connection> conn)
            :
            Network::Session(std::move(conn)) {
        }

        void OnConnected() override {
            session_id_ = session_id_counter_++;

            LOG_INFO("ClientSession::OnConnect sesion_id:{}", session_id_);

            SendMessage("Hello, server!");

            InstallProtobuf();
        }

        void OnDisconnected() override {
            LOG_INFO("ClientSession::OnDisconnect");
        }

        void OnMessage(const std::string& message) override {
            LOG_INFO("ClientSession::OnMessage: {}", message.c_str());
        }

        int32_t session_id() const { return session_id_; }

    private:
        int session_id_;
        static int session_id_counter_;

        void InstallProtobuf();
    };
}