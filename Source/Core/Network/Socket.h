#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace System {
	class Context;
} // namespace System

namespace Network {
class Connection;
class Socket {
public:
	using ConnectCallback = void (Connection::*)(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&);
	using ReadCallback = void (Connection::*)(const boost::system::error_code&, std::size_t);

	Socket(const std::shared_ptr<System::Context>& context);
	~Socket();

	bool IsOpen() const;
	void Close();

	template<typename Option>
	void SetOption(const Option& option){
		socket_->set_option(option);
	}

	void ConnectAsync(const boost::asio::ip::tcp::resolver::results_type& results, ConnectCallback callback, std::shared_ptr<Connection> conn);
	void WriteAsync(std::shared_ptr<std::string> message);
	void ReadAsync(std::string& buffer, ReadCallback callback, std::shared_ptr<Connection> conn);

	std::string address() const { return socket_->remote_endpoint().address().to_string(); }
	uint16_t port() const { return socket_->remote_endpoint().port(); }

	std::shared_ptr<System::Context>& context() { return context_; }
	const std::shared_ptr<System::Context>& context() const { return context_; }

	boost::asio::ip::tcp::socket& socket() { return *socket_; }
	const boost::asio::ip::tcp::socket& socket() const { return *socket_; }

private:
	std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
	std::shared_ptr<System::Context> context_;
};
}  // namespace Network