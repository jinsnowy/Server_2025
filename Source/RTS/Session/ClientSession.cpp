#include "stdafx.h"
#include "ClientSession.h"
#include "Protobuf/Protobuf.h"
#include "PacketHandler/ClientPacketHandler.h"

namespace RTS {

	int ClientSession::session_id_counter_ = 1;

	void ClientSession::InstallProtobuf() {
		InstallProtocol(std::make_unique<ProtobufProtocol>(&ClientPacketHandler::GetInstance()));
	}

}