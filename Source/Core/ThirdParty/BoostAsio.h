#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

namespace Network {
struct WSAInitializer {
	WSAInitializer();
	~WSAInitializer();
};
} // namespace Network

extern Network::WSAInitializer wsa_initializer;

#endif

#include <boost/asio.hpp>