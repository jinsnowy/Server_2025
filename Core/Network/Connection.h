#pragma once

#include <boost/asio.hpp>

namespace Network {
	class Session;
	class SessionFactory;
	class Connection final : public std::enable_shared_from_this<Connection> {
	public:
		Connection(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket);
		Connection(boost::asio::io_context& io_context);
		~Connection();

		void Connect(const std::string& ip, const uint16_t& port, std::function<bool(std::shared_ptr<Connection>)> on_connect);
		void Disconnect();
		void Send(std::string message);
		bool IsConnected() const;

		void SetSession(std::weak_ptr<Session> session);
		boost::asio::io_context& io_context() const;

		std::string ToString() const;

	private:
		boost::asio::io_context& io_context_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::resolver resolver_;
		std::string ip_;
		uint16_t port_;
		std::string buffer_;
		std::weak_ptr<Session> session_;
		void Receive();
	};
}
