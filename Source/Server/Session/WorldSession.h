#pragma once

#include "Protobuf/Public/ProtobufSession.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf


namespace Server {
	class WorldHandlerMap;
	class WorldSession : public Protobuf::ProtobufSession {
	public:
		WorldSession();
		~WorldSession();

		static void RegisterHandler(WorldHandlerMap* handler_map);

		void OnConnected();

		void OnDisconnected();

		std::unique_ptr<Network::Protocol> CreateProtocol() override;

		int64_t session_id() const { return session_id_; }

	private:
		int64_t session_id_ = 0;
	};
}

