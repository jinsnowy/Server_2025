#include "stdafx.h"
#include "Connection.h"
#include "Core/Network/Session.h"

namespace Network {
	Connection::Connection(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket)
	: 
	io_context_(io_context), socket_(std::move(socket)), resolver_(io_context) {
	}

	Connection::Connection(boost::asio::io_context& io_context)
		: 
		io_context_(io_context), socket_(io_context), resolver_(io_context) {
	}

	Connection::~Connection() {
		Disconnect();
	}

	void Connection::Connect(const std::string& ip, const uint16_t& port, std::function<bool(std::shared_ptr<Connection>)> on_connect) {
		ip_ = ip;
		port_ = port;

		resolver_.async_resolve(ip_, std::to_string(port_), [conn = shared_from_this(), on_connect](const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results) {
			if (error) {
				LOG_INFO("[CONNECTION] resolve to {} failed {}", conn->GetConnectionString(), error.to_string());
				return;
			}

			boost::asio::async_connect(conn->socket_, results, [conn, on_connect](const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
				if (error){
					LOG_INFO("[CONNECTION] connect to {} failed {}", conn->GetConnectionString(), error.to_string());
					return;
				}

				conn->socket_.set_option(boost::asio::socket_base::keep_alive(true));
				conn->socket_.set_option(boost::asio::socket_base::linger(true, 0));

				conn->ip_ = endpoint.address().to_string();
				conn->port_ = endpoint.port();

				if (on_connect(conn) == true){
					conn->Receive();
				}
			});
		});
	}

	void Connection::Disconnect() {
		if (socket_.is_open()){
			socket_.close();

			auto session = session_.lock();
			if (session){
				session->OnDisconnect();
			}
		}
	}

	bool Connection::IsConnected() const {
		return socket_.is_open();
	}

	void Connection::Send(std::string message) {
		boost::asio::post(io_context_, [message = std::move(message), weak_conn = weak_from_this()]() {
			auto conn = weak_conn.lock();
			if (conn == nullptr){
				return;
			}
			boost::asio::write(conn->socket_, boost::asio::buffer(message));
		});
	}

	void Connection::Receive() {
		buffer_.reserve(1024);

		boost::asio::post(io_context_, [weak_conn = weak_from_this()]() {
			auto conn = weak_conn.lock();
			if (conn == nullptr){
				return;
			}

			boost::asio::async_read(conn->socket_, 
			boost::asio::buffer(conn->buffer_), 
			boost::asio::transfer_at_least(1), 
			[weak_conn](const boost::system::error_code& error, std::size_t bytes_transferred) {
				if (error){
					return;
				}

				auto conn = weak_conn.lock();
				if (conn == nullptr){
					return;
				}

				if (bytes_transferred == 0){
					conn->Disconnect();
					return;
				}

				try {
					conn->buffer_.resize(bytes_transferred);

					auto session = conn->session_.lock();
					if (session){
						session->OnReceive(conn->buffer_);
					}
				}
				catch (const std::exception& e){
					LOG_ERROR("Connection::Receive() error: {}", e.what());
					conn->Disconnect();
				}
			});
		});
	}

	void Connection::SetSession(std::weak_ptr<Session> session) {
		session_ = session;
	}

	boost::asio::io_context& Connection::io_context() const {
		return io_context_;
	}

	std::string Connection::ToString() const {
		return FORMAT("{}:{}", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
	}

	std::string Connection::GetConnectionString() const {
		return FORMAT("{}:{}", ip_, port_);
	}
}
