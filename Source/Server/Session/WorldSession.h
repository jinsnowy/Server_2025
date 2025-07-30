#pragma once

#include "Protobuf/Public/ProtobufSession.h"
#include "Server/Model/ClientAction.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf


namespace Server {
	namespace Model {
		class Player;
	}

	class Section;
	class Pc;
	class WorldHandlerMap;
	class WorldService;
	class WorldSession : public Protobuf::ProtobufSession {
	public:
		WorldSession();
		~WorldSession();

		static void RegisterHandler(WorldHandlerMap* handler_map);

		static WorldService& GetService();

		void OnConnected();
		void OnDisconnected();

		std::unique_ptr<Network::Protocol> CreateProtocol() override;

		int64_t character_id() const { return character_id_; }
		void set_character_id(int64_t character_id) { character_id_ = character_id; }

		int16_t server_id() const { return server_id_; }

		void OnSectionTick(float delta_time);

		void set_player(std::unique_ptr<Model::Player> player);

		const Model::Player& player() const {
			DEBUG_ASSERT(player_ != nullptr);
			return *player_;
		}

		Model::Player& player() {
			DEBUG_ASSERT(player_ != nullptr);
			return *player_;
		}

		const Pc& pc() const {
			return *pc_;
		}

		Pc& pc() {
			return *pc_;
		}

		std::shared_ptr<Pc> GetPc() const {
			return pc_;
		}

		const std::shared_ptr<Section>& section() const {
			return section_;
		}

		void OnSectionEntered(std::shared_ptr<Section> section);
		void OnSectionLeft(std::shared_ptr<Section> section);

		void OnVerifyClientAction(const std::shared_ptr<const world::ClientActionReq>& msg, world::ClientActionRes& res);

		int64_t IssueAction(const types::ClientAction::ClientActionFieldCase& type, int32_t expire_ms);
		bool ConsumeAction(const types::ClientAction::ClientActionFieldCase& type, Model::ClientAction* out_action);

		int64_t GenerateId() const;
		

	private:
		int64_t character_id_ = 0;
		int16_t server_id_ = 0;
		std::unique_ptr<Model::Player> player_;
		std::shared_ptr<Pc> pc_;
		std::shared_ptr<Section> section_; // Weak reference to the section this session is in
		std::list<Model::ClientAction> action_queue_; // Queue of actions to be processed
	};
}

