#pragma once
#include "Core/System/Singleton.h"

namespace Network {
	class Session;
	template<typename TPacket>
	class PacketHandlerMap {
	public:
		using Type = PacketHandlerMap<THandler, TPacket, TPacketSerializer>;
		using HandlerFunc = std::function<void(THandler&, const std::shared_ptr<const TPacket>&)>;

		template<typename TSession, typename TMessage, typename = std::enable_if_t<std::is_base_of_v<TPacket, TMessage>>>
		using TypedHandlerFuncPointer = void(*)(TSession&, const std::shared_ptr<const TMessage>&);

	public:
		template<typename T>
		void RegisterHandler(size_t packetId, TypedHandlerFuncPointer<T>&& typedHandler) {
			static_assert(std::is_base_of_v<TPacket, T>, "T must be derived from TPacket" );

			_handlers[packetId] = [typedHandler](THandler& handler, const std::shared_ptr<const TPacket>& packet) {
				typedHandler(handler, std::static_pointer_cast<const T>(packet));
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