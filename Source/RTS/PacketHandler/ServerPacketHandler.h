#pragma once

#include "Core/Network/Packet/PacketHandlerMap.h"

namespace RTS {
	class ServerPacketHandler : public Network::PacketHandlerMap<google::protobuf::Message>, public System::Singleton<ServerPacketHandler>{
	public:

	};
}


