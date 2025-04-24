#include "stdafx.h"
#include "BoostAsio.h"

namespace Network {
WSAInitializer::WSAInitializer() {
	WSAData wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		throw std::runtime_error("Failed to initialize Winsock");
	}
}

WSAInitializer::~WSAInitializer() {
	WSACleanup();
}

WSAInitializer wsa_initializer = {};
	
}
