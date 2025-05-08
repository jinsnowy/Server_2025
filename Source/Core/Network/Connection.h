#pragma once

#include "Core/ThirdParty/BoostAsio.h"
#include "Core/System/Actor.h"

namespace System{
	class Context;
} // namespace System

namespace Network {
	class Session;
	class SessionFactory;
	class Socket;
	class Resolver;
	class Session;
	class Buffer;
	class Connection final : public System::Actor<Connection> {
	public:
		Connection(std::unique_ptr<Socket> socket);
		Connection(std::shared_ptr<System::Context> context);
		~Connection();

		void Connect(const std::string& ip, const uint16_t& port, std::function<bool(std::shared_ptr<Connection>)> on_connect);
		void Disconnect();
		void Send(const Buffer& buffer);
		bool IsConnected() const;

		void BeginSession(std::shared_ptr<Session> session);
		void BeginReceive(std::shared_ptr<char[]> buffer, int32_t size);

		std::string ToString() const;

	private:
		std::unique_ptr<Socket> socket_;
		std::unique_ptr<Resolver> resolver_;
		std::weak_ptr<Session> session_;
		std::function<bool(std::shared_ptr<Connection>)> on_connect_;

		std::string ip_;
		uint16_t port_ = 0;

		void OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results);
		void OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint);
		void OnReceived(const boost::system::error_code& error, std::size_t bytes_transferred);
		void OnSendCompleted(const boost::system::error_code& error, std::size_t bytes_transferred);
	};
}
