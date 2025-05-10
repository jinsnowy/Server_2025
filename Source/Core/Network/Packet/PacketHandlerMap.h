#pragma once
#include "Core/System/Singleton.h"

namespace Network {
	class Session;
	template<typename TPacket>
	class PacketHandlerMap {
	public:
		virtual ~PacketHandlerMap() = default;

		using HandlerFunc = std::function<void(Session&, const std::shared_ptr<const TPacket>&)>;

		template<typename TSession, typename TMessage, typename = std::enable_if_t<std::is_base_of_v<TPacket, TMessage>>>
		using TypedHandlerFuncPointer = void(*)(TSession&, const std::shared_ptr<const TMessage>&);

	public:
		template<typename TSession, typename TMessage>
		void RegisterHandler(size_t packetId, TypedHandlerFuncPointer<TSession, TMessage> typedHandler) {
			static_assert(std::is_base_of_v<TPacket, TMessage>, "TMessage must be derived from TPacket");
			static_assert(std::is_base_of_v<Session, TSession>, "TSession must be derived from Session");

			_handlers[packetId] = [typedHandler](Session& handler, const std::shared_ptr<const TPacket>& packet) {
				typedHandler(static_cast<TSession&>(handler), std::static_pointer_cast<const TMessage>(packet));
			};
		}

		bool HandleMessage(Session& session, const size_t packetId, const std::shared_ptr<const TPacket>& message) {
			auto iter = _handlers.find(packetId);
			if (iter == _handlers.end()) {
				return false;
			}
			(iter->second)(session, message);
			return true;
		}

	private:
		std::unordered_map<size_t, HandlerFunc> _handlers;
	};
}