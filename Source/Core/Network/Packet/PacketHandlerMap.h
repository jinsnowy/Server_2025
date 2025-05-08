#pragma once

namespace Network {

	template<typename THandler, typename TPacket, typename TPacketSerializer>
	class PacketHandlerMap {
	public:
		using Type = PacketHandlerMap<THandler, TPacket, TPacketSerializer>;
		using HandlerFunc = std::function<void(THandler&, const std::shared_ptr<const TPacket>&)>;

		template<typename T, typename = std::enable_if_t < std::is_base_of_v<TPacket, T>>>
		using TypedHandlerFuncPointer = void(*)(THandler&, const std::shared_ptr<const T>&);

	public:
		template<typename T>
		void RegisterHandler(size_t packetId, TypedHandlerFuncPointer<T>&& typedHandler) {
			static_assert(std::is_base_of_v<TPacket, T>, "T must be derived from TPacket" );

			_handlers[packetId] = [typedHandler](THandler& handler, const std::shared_ptr<const TPacket>& packet) {
				typedHandler(handler, std::static_pointer_cast<const T>(packet));
			};
		}

		bool HandleMessage(THandler& receiver, const std::shared_ptr<const TPacket>& message) {
			const size_t packetId = _serializer.Resolve(*message);
			const auto it = _handlers.find(packetId);
			if (it == _handlers.end()) {
				return false;
			}
			it->second(receiver, message);
			return true;
		}

	private:
		TPacketSerializer _serializer;
		std::unordered_map<size_t, HandlerFunc> _handlers;
	};
}