#pragma once

#include "Core/Network/ProtocolController.h"
#include "Protobuf/Public/ProtobufSerializer.h"
#include "Protobuf/Public/ProtobufDescriptor.h"

namespace Protobuf {
	using ProtobufProtocol = Network::ProtocolController<google::protobuf::Message, ProtobufSerializer>;

	class ProtobufHandlerMap : public Network::PacketHandlerMap<google::protobuf::Message> {
	public:
		template<typename TSession, typename TMessage>
		void Register(void(*handler)(TSession&, const std::shared_ptr<const TMessage>&)) {
			const auto* descriptor = ProtobufDescriptor::GetDescriptor<TMessage>();
			const size_t messageId = ProtobufDescriptor::GetMessageId(descriptor);
			ProtobufDescriptor::RegisterDescriptor(messageId, descriptor);
			RegisterHandler<TSession, TMessage>(messageId, handler);
		}
	};
}