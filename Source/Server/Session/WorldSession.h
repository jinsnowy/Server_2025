#pragma once

#include "Protobuf/Public/ProtobufSession.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf


namespace Server {
	namespace Model {
		class Player;
	}

	class Section;
	class WorldHandlerMap;
	class WorldSession : public Protobuf::ProtobufSession {
	public:
		WorldSession();
		~WorldSession();

		static void RegisterHandler(WorldHandlerMap* handler_map);

		void OnConnected();
		void OnDisconnected();

		std::unique_ptr<Network::Protocol> CreateProtocol() override;

		int64_t character_id() const { return character_id_; }
		void set_character_id(int64_t character_id) { character_id_ = character_id; }

		void OnMovableTick(const System::Tick& tick);

		void set_player(std::unique_ptr<Model::Player> player);

		const Model::Player* player() const {
			return player_.get();
		}

		Model::Player* player() {
			return player_.get();
		}

		void OnSectionEntered(std::shared_ptr<Section> section);
		void OnSectionLeft(std::shared_ptr<Section> section);

	private:
		int64_t character_id_ = 0;
		std::unique_ptr<Model::Player> player_;
		std::shared_ptr<Section> section_; // Weak reference to the section this session is in
	};
}

