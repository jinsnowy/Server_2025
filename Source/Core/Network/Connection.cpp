#include "stdafx.h"
#include "Connection.h"
#include "Core/Network/Session.h"
#include "Core/System/Scheduler.h"
#include "Core/System/Channel.h"
#include "Core/Network/Resolver.h"
#include "Core/Network/Socket.h"

namespace Network {
	Connection::Connection(std::unique_ptr<Socket> socket) 
		:
		System::Actor<Connection>(System::Channel(socket->context())),
		socket_(std::move(socket)),
		resolver_(std::make_unique<Resolver>(socket_->context())) {
	}

	Connection::Connection(std::shared_ptr<System::Context> context)
		:
		System::Actor<Connection>(System::Channel(context)),
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
				session->OnDisconnect();
				session_.reset();
			}
		}
	}

	bool Connection::IsConnected() const {
		return socket_->IsOpen();
	}

	void Connection::Send(std::string message) {
		Post([message = std::move(message)](Connection& connection) {
			auto message_shared = std::make_shared<std::string>(std::move(message));
			connection.socket_->WriteAsync(message_shared);
		});
	}

	void Connection::OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] resolve to {} failed {}", ToString(), error.to_string());
			return;
		}

		socket_->ConnectAsync(results, &Connection::OnConnected, shared_from_this());
	}

	void Connection::OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] connect to {} failed {}", ToString(), error.to_string());
			return;
		}

		socket_->SetOption(boost::asio::socket_base::keep_alive(true));
		socket_->SetOption(boost::asio::socket_base::linger(true, 0));

		ip_ = endpoint.address().to_string();
		port_ = endpoint.port();

		if (on_connect_(shared_from_this()) == false) {
			Disconnect();
		}
	}

	void Connection::BeginReceive() {
		DEBUG_ASSERT(IsSynchronized());
		buffer_.resize(1024);
		socket_->ReadAsync(buffer_, &Connection::OnReceived, shared_from_this());
	}

	void Connection::OnReceived(const boost::system::error_code& error, std::size_t bytes_transferred) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("Error during async_read: {}", error.message());
			return;
		}

		if (bytes_transferred == 0) {
			Disconnect();
			return;
		}

		try {
			buffer_.resize(bytes_transferred);

			auto session = session_.lock();
			if (session) {
				session->OnReceive(buffer_);
			}

			BeginReceive();
		}
		catch (const std::exception& e) {
			LOG_ERROR("Connection::Receive() error: {}", e.what());
			Disconnect();
		}
	}

	void Connection::BeginSession(std::weak_ptr<Session> session) {
		DEBUG_ASSERT(IsSynchronized());
		session_ = session;
		try {
			BeginReceive();
			ip_ = socket_->address();
			port_ = socket_->port();
		}
		catch (const std::exception& e) {
			LOG_ERROR("Connection::SetSession() error: {}", e.what());
			Disconnect();
		}
	}

	std::string Connection::ToString() const {
		return FORMAT("{}:{}", ip_, port_);
	}
}
