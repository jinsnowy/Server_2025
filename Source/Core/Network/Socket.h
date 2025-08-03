#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace System {
	class Context;
} // namespace System

namespace Network {
class BufferMemory;
class BufferView;
class Buffer;
class Connection;
class Session;
class Socket {
public:
	using ConnectCallback = void (Connection::*)(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&);
	using ReadCallback = void (Connection::*)(const boost::system::error_code&, std::size_t);
	using SendCallback = void (Connection::*)(const boost::system::error_code&, const std::shared_ptr<Buffer>&, std::size_t);

	Socket(const std::shared_ptr<System::Context>& context);
	~Socket();

	bool IsOpen() const;
	void Close();

	void ConnectAsync(const boost::asio::ip::tcp::resolver::results_type& results, ConnectCallback callback, std::shared_ptr<Connection> conn);
	void WriteAsync(const BufferView& buffer, SendCallback callback, std::shared_ptr<Connection> conn);
	void ReadAsync(Buffer& buffer, ReadCallback callback, std::shared_ptr<Connection> conn);

	std::string address() const { return socket_->remote_endpoint().address().to_string(); }
	uint16_t port() const { return socket_->remote_endpoint().port(); }

	std::shared_ptr<System::Context>& context() { return context_; }
	const std::shared_ptr<System::Context>& context() const { return context_; }

	boost::asio::ip::tcp::socket& raw_socket() { return *socket_; }
	const boost::asio::ip::tcp::socket& raw_socket() const { return *socket_; }

private:
	std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
	std::shared_ptr<System::Context> context_;
};
}  // namespace Network