#pragma once
#include "Core/System/Singleton.h"

namespace Network {
	class Session;
	template<typename TPacket>
	class PacketHandlerMap {
	public:
		using Type = PacketHandlerMap<TPacket>;
		using HandlerFunc = std::function<void(Session&, const std::shared_ptr<const TPacket>&)>;
		using Packet = TPacket;

		template<typename TSession, typename TMessage, typename = std::enable_if_t<std::is_base_of_v<TPacket, TMessage>>>
		using TypedHandlerFuncPointer = void(*)(TSession&, const std::shared_ptr<const TMessage>&);

	public:
		PacketHandlerMap() = default;
		~PacketHandlerMap() = default;

		template<typename TSession, typename TMessage>
		void RegisterHandler(size_t packetId, TypedHandlerFuncPointer<TSession, TMessage>&& typedHandler) {
			_handlers[packetId] = [typedHandler = std::forward<TypedHandlerFuncPointer<TSession, TMessage>>(typedHandler)](TSession& handler, const std::shared_ptr<const TPacket>& packet) mutable {
				typedHandler(handler, std::static_pointer_cast<const TMessage>(packet));
			};
		}

		bool HandleMessage(Session& session, const size_t packetId, const std::shared_ptr<const TPacket>& message) {
			auto iter = _handlers.find(packetId);
			if (iter == _handlers.end()) {
				return false;
			}
			(*iter)(session, message);
			return true;
		}

	private:
		std::unordered_map<size_t, HandlerFunc> _handlers;
	};
}