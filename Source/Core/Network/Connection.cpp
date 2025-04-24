#include "stdafx.h"
#include "Connection.h"
#include "Core/Network/Session.h"
#include "Core/System/Scheduler.h"

namespace Network {
	Connection::Connection(boost::asio::ip::tcp::socket socket)
		: 
		io_context_(static_cast<boost::asio::io_context&>(socket.get_executor().context())),
		resolver_(static_cast<boost::asio::io_context&>(socket.get_executor().context())),
		socket_(std::move(socket)) {
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

		System::Scheduler::Get(io_context_).Post([weak_conn = weak_from_this(), on_connect = std::move(on_connect)]() mutable {
			auto conn = weak_conn.lock();
			if (conn == nullptr) {
				return;
			}

			conn->resolver_.async_resolve(conn->ip_, std::to_string(conn->port_), [weak_conn, on_connect = std::move(on_connect)](const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results) {
				auto conn = weak_conn.lock();
				if (conn == nullptr) {
					return;
				}

				if (error) {
					LOG_ERROR("[CONNECTION] resolve to {} failed {}", conn->GetConnectionString(), error.to_string());
					return;
				}

				boost::asio::async_connect(conn->socket_, results, [weak_conn, on_connect](const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
						auto conn = weak_conn.lock();
						if (conn == nullptr) {
							return;
						}
					
						if (error) {
							LOG_ERROR("[CONNECTION] connect to {} failed {}", conn->GetConnectionString(), error.to_string());
							return;
						}

						conn->socket_.set_option(boost::asio::socket_base::keep_alive(true));
						conn->socket_.set_option(boost::asio::socket_base::linger(true, 0));

						conn->ip_ = endpoint.address().to_string();
						conn->port_ = endpoint.port();

						if (on_connect(conn) == false) {
							conn->Disconnect();
						}
					});
				});
		});
	}

	void Connection::Disconnect() {
		if (socket_.is_open()){
			socket_.close();

			auto session = session_.lock();
			if (session){
				session->OnDisconnect();
				session_.reset();
			}
		}
	}

	bool Connection::IsConnected() const {
		return socket_.is_open();
	}

	void Connection::Send(std::string message) {
		boost::asio::post(io_context_, [message = std::move(message), weak_conn = std::weak_ptr(shared_from_this())]() {
			auto conn = weak_conn.lock();
			if (conn == nullptr) {
				return;
			}
			bool is_connected = conn->IsConnected();
			UNREFERENCED_PARAMETER(is_connected);

			auto message_shared = std::make_shared<std::string>(std::move(message));
			boost::asio::async_write(conn->socket_, boost::asio::buffer(*message_shared),
				[weak_conn, message_shared](const boost::system::error_code& error, std::size_t bytes_transferred) {
					auto conn = weak_conn.lock();
					if (conn == nullptr) {
						return;
					}

					if (error) {
						if (error == boost::asio::error::operation_aborted) {
							return;  // 정상적인 취소
						}
						LOG_ERROR("Send failed :{}, {}", error.message(), error.value());
						conn->Disconnect();
					}

					UNREFERENCED_PARAMETER(bytes_transferred);
				});
			});
	}

	void Connection::StartReceive() {
		boost::asio::post(io_context_, [weak_conn = weak_from_this()]() {
			auto conn = weak_conn.lock();
			if (conn == nullptr) {
				return;
			}
			conn->buffer_.resize(1024);
			boost::asio::async_read(conn->socket_,
			boost::asio::buffer(conn->buffer_),
			boost::asio::transfer_at_least(1),
			[weak_conn](const boost::system::error_code& error, std::size_t bytes_transferred) {
				auto conn = weak_conn.lock();
				if (conn == nullptr) {
					return;
				}

				if (error) {
					LOG_ERROR("Error during async_read: {}", error.message());
					return;
				}

				if (bytes_transferred == 0) {
					conn->Disconnect();
					return;
				}

				try {
					conn->buffer_.resize(bytes_transferred);

					auto session = conn->session_.lock();
					if (session) {
						session->OnReceive(conn->buffer_);
					}

					conn->StartReceive();
				}
				catch (const std::exception& e) {
					LOG_ERROR("Connection::Receive() error: {}", e.what());
					conn->Disconnect();
				}
			});
		});
	}

	void Connection::SetSession(std::weak_ptr<Session> session) {
		session_ = session;
		try {
			StartReceive();
			ip_ = socket_.local_endpoint().address().to_string();
			port_ = socket_.local_endpoint().port();
		}
		catch (const std::exception& e) {
			LOG_ERROR("Connection::SetSession() error: {}", e.what());
			Disconnect();
		}
	}

	boost::asio::io_context& Connection::io_context() const {
		return io_context_;
	}

	std::string Connection::ToString() const {
		return GetConnectionString();
	}

	std::string Connection::GetConnectionString() const {
		return FORMAT("{}:{}", ip_, port_);
	}
}
