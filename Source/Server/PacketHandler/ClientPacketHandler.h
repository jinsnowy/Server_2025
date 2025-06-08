#pragma once

#include "Core/Network/Packet/PacketHandlerMap.h"

namespace Server {

	class ClientPacketHandler : public Network::PacketHandlerMap<google::protobuf::Message>, public System::Singleton<ClientPacketHandler> {
	public:

	};
}