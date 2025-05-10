#include "stdafx.h"
#include "Connection.h"
#include "Core/Network/Session.h"
#include "Core/System/Scheduler.h"
#include "Core/System/Channel.h"
#include "Core/Network/Resolver.h"
#include "Core/Network/Socket.h"
#include "Core/Network/Buffer.h"

namespace Network {
	Connection::Connection(std::unique_ptr<Socket> socket) 
		:
		System::Actor<Connection>(System::Channel(socket->context())),
		socket_(std::move(socket)),
		resolver_(std::make_unique<Resolver>(socket_->context())) {
	}

	Connection::Connection(std::shared_ptr<System::Context> context)
		:
		socket_(std::make_unique<Socket>(context)),
		resolver_(std::make_unique<Resolver>(context)) {
	}

	Connection::~Connection() {
		if (IsConnected()) {
			Disconnect();
		}
	}

	void Connection::Connect(const std::string& ip, const uint16_t& port, std::function<bool(std::shared_ptr<Connection>)> on_connect) {
		DEBUG_ASSERT(IsSynchronized());

		if (on_connect == nullptr) {
			throw std::runtime_error("on_connect callback is not set");
		}

		ip_ = ip;
		port_ = port;
		on_connect_ = std::move(on_connect);
		resolver_->Resolve(ip_, port_, &Connection::OnResolved, shared_from_this());
	}

	void Connection::Disconnect() {
		DEBUG_ASSERT(IsSynchronized());
		if (socket_->IsOpen()){
			socket_->Close();

			auto session = session_.lock();
			if (session){
				session->OnDisconnected();
				session_.reset();
			}
		}
	}

	bool Connection::IsConnected() const {
		return socket_->IsOpen();
	}

	void Connection::Send(const Buffer& buffer) {
		DEBUG_ASSERT(IsSynchronized());
		socket_->WriteAsync(buffer.buffer_shared(), buffer.start_pos(), buffer.GetByteCount(), &Connection::OnSendCompleted, shared_from_this());
	}

	void Connection::OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] resolve to {} failed by {}", ToString(), error.message());
			return;
		}

		socket_->ConnectAsync(results, &Connection::OnConnected, shared_from_this());
	}

	void Connection::OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] connect to {} failed by {}", ToString(), error.to_string());
			return;
		}

		socket_->socket().set_option(boost::asio::socket_base::keep_alive(true));
		socket_->socket().set_option(boost::asio::socket_base::linger(true, 0));

		ip_ = endpoint.address().to_string();
		port_ = endpoint.port();

		if (on_connect_(shared_from_this()) == false) {
			Disconnect();
		}
	}

	void Connection::BeginReceive(std::shared_ptr<char[]> buffer, int32_t size) {
		DEBUG_ASSERT(IsSynchronized());
		if (!buffer || size <= 0) {
			LOG_ERROR("[CONNECTION] BeginReceive: invalid buffer");
			return;
		}

		socket_->ReadAsync(buffer, size, &Connection::OnReceived, shared_from_this());
	}

	void Connection::OnReceived(const boost::system::error_code& error, std::size_t bytes_transferred) {
		DEBUG_ASSERT(IsSynchronized());

		if (socket_->IsOpen() == false) {
			Disconnect();
			return;
		}

		if (bytes_transferred == 0) {
			Disconnect();
			return;
		}

		if (error) {
			Disconnect();
			LOG_ERROR("Error during async_read: error_code: {}, message: {}", error.value(), error.message());
			return;
		}

		try {
			auto session = session_.lock();
			if (session == nullptr) {
				Disconnect();
				return;
			}

			if (session->OnReceived(bytes_transferred) == false) {
				LOG_ERROR("[CONNECTION] OnReceived failed");
				Disconnect();
				return;
			}

			session->BeginReceive();
		}
		catch (const std::exception& e) {
			LOG_ERROR("Connection::Receive() error: {}", e.what());
			Disconnect();
		}
	}

	void Connection::OnSendCompleted(const boost::system::error_code& error, std::size_t bytes_transferred) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("Error during async_write: {}", error.message());
		}

		auto session = session_.lock();
		if (session == nullptr) {
			Disconnect();
			return;
		}

		session->FlushSend(true);
	}

	void Connection::BeginSession(std::shared_ptr<Session> session) {
		DEBUG_ASSERT(IsSynchronized());
		session_ = session;
		ip_ = socket_->address();
		port_ = socket_->port();
	}

	std::string Connection::ToString() const {
		return FORMAT("{}:{}", ip_, port_);
	}
}
