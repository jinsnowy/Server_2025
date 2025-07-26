#include "stdafx.h"
#include "Socket.h"
#include "Core/System/Context.h"
#include "Core/Network/Connection.h"
#include "Core/Network/BufferMemory.h"
#include "Core/Network/Buffer.h"

namespace Network {
	Socket::Socket(const std::shared_ptr<System::Context>& context)
		:
		context_(context),
		socket_(std::make_unique<boost::asio::ip::tcp::socket>(context->io_context())) {
	}

	Socket::~Socket() {
	}

	bool Socket::IsOpen() const {
		return socket_->is_open();
	}

	void Socket::Close() {
		try {
			socket_->close();
		}catch (const boost::system::system_error& e) {
			UNREFERENCED_PARAMETER(e);
			//LOG_ERROR("[SOCKET] close error: {}", e.what());
		}
	}

	struct ConnectCompletionEvent {
		std::weak_ptr<Connection> weak_conn;
		Socket::ConnectCallback callback;

		void operator()(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
			auto conn = weak_conn.lock();
			if (!conn) {
				return;
			}
			if (error == boost::asio::error::operation_aborted) {
				return;
			}
			if (error) {
				LOG_ERROR("[SOCKET] connect error: {}, endpoint: {}", error.message(), endpoint.address().to_string());
				return;
			}
		
			Ctrl(*conn).Post([callback=this->callback, error, endpoint](Connection& conn) {
				(conn.*callback)(error, endpoint);
			});
		}
	};

	void Socket::ConnectAsync(const boost::asio::ip::tcp::resolver::results_type& results, ConnectCallback callback, std::shared_ptr<Connection> conn) {
		try {
			boost::asio::async_connect(*socket_, results, ConnectCompletionEvent{conn, callback });
		}catch (const boost::system::system_error& e) {
			LOG_ERROR("[SOCKET] connect error: {}", e.what());
		}
		catch (const std::exception& e) {
			LOG_ERROR("[SOCKET] connect error: {}", e.what());
		}
		catch (...) {
			LOG_ERROR("[SOCKET] connect error: unknown error");
		}
	}

	struct WriteCompletionEvent {
		Socket::SendCallback callback;
		std::weak_ptr<Connection> weak_conn;
		std::shared_ptr<BufferMemory> buffer;

		void operator()(const boost::system::error_code& error, std::size_t bytes_transferred) {
			auto conn = weak_conn.lock();
			if (!conn) {
				return;
			}
			Ctrl(*conn).Post([callback = this->callback, error, bytes_transferred](Connection& conn) {
				(conn.*callback)(error, bytes_transferred);
			});
		}
	};

	void Socket::WriteAsync(const BufferView& buffer, SendCallback callback, std::shared_ptr<Connection> conn) {
		try {
			std::string_view buffer_view(buffer.data(), buffer.length());
			boost::asio::async_write(*socket_, boost::asio::buffer(buffer_view), WriteCompletionEvent{ callback, conn, buffer.source()});
			//LOG_INFO("[SOCKET] write async: {} bytes", buffer.length());
		}
		catch (const boost::system::system_error& e) {
			LOG_ERROR("[SOCKET] write error: {}", e.what());
		}
		catch (const std::exception& e) {
			LOG_ERROR("[SOCKET] write error: {}", e.what());
		}
		catch (...) {
			LOG_ERROR("[SOCKET] write error: unknown error");
		}
	}

	void Socket::ReadAsync(Buffer& buffer, ReadCallback callback, std::shared_ptr<Connection> conn) {
		try {
			boost::asio::async_read(*socket_,
			boost::asio::buffer(buffer.GetBufferPtr(), buffer.GetBufferSize()),
			boost::asio::transfer_at_least(1),
			[buffer_shared = buffer.source(), callback, weak_conn = std::weak_ptr(conn)](const boost::system::error_code& error, std::size_t bytes_transferred) {
				auto conn = weak_conn.lock();
				if (conn == nullptr) {
					return;
				}

				Ctrl(*conn).Post([callback, error, bytes_transferred](Connection& conn) {
					(conn.*callback)(error, bytes_transferred);
				});

				//LOG_INFO("[SOCKET] read async: {} bytes", bytes_transferred);
			});
		}
		catch (const boost::system::system_error& e) {
			LOG_ERROR("[SOCKET] read error: {}", e.what());
		}
		catch (const std::exception& e) {
			LOG_ERROR("[SOCKET] read error: {}", e.what());
		}
		catch (...) {
			LOG_ERROR("[SOCKET] read error: unknown error");
		}	
	}
}
