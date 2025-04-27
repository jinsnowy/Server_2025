#pragma once

namespace Network {

	template<typename THandler, typename TPacket, typename TPacketSerializer>
	class PacketHandlerMap {
	public:
		using Type = PacketHandlerMap<THandler, TPacket, TPacketSerializer>;
		using HandlerFunc = std::function<void(THandler&, const std::shared_ptr<const TPacket>&)>;
		using Packet = TPacket;
		using Serializer = TPacketSerializer;
		using Handler = THandler;

		template<typename T, typename = std::enable_if_t < std::is_base_of_v<TPacket, T>>>
		using TypedHandlerFuncPointer = void(*)(THandler&, const std::shared_ptr<const T>&);

	public:
		PacketHandlerMap();
		virtual ~PacketHandlerMap() = default;

		template<typename T>
		void RegisterHandler(size_t packetId, TypedHandlerFuncPointer<T>&& typedHandler) {
			_handlers[packetId] = [typedHandler = std::forward<TypedHandlerFuncPointer<T>>(typedHandler)](THandler& handler, const std::shared_ptr<const TPacket>& packet) mutable {
				typedHandler(handler, std::static_pointer_cast<const T>(packet));
			};
		}

		void HandleMessage(THandler& receiver, const std::shared_ptr<const TPacket>& message) {
			size_t packetId = _serializer.Resolve(*message);
			_handlers[packetId](receiver, message);
		}

	private:
		Serializer _serializer;
		std::unordered_map<size_t, HandlerFunc> _handlers;
	};

	template<typename THandler, typename TPacket, typename TPacketSerializer>
	inline PacketHandlerMap<THandler, TPacket, TPacketSerializer>::PacketHandlerMap()
	{
	}
}