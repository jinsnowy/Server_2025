#include "stdafx.h"
#include "Acceptor.h"
#include "Core/System/Context.h"
#include "Core/System/Scheduler.h"
#include "Core/Network/Socket.h"
#include "Core/Network/Listener.h"

namespace Network {

	Acceptor::Acceptor(std::shared_ptr<System::Context> context)
		:
		context_(context),
		acceptor_(std::make_unique<boost::asio::ip::tcp::acceptor>(context_->io_context())) {
	}

	Acceptor::~Acceptor() = default;

	void Acceptor::Bind(std::string ip, uint16_t port) {
		endpoint_ = std::make_unique<boost::asio::ip::tcp::endpoint>(boost::asio::ip::make_address(ip), port);
		acceptor_->open(endpoint_->protocol());
		acceptor_->bind(*endpoint_);
	}

	void Acceptor::Listen() {
		acceptor_->listen();
		LOG_INFO("Listening ... {}", ToString());
	}

	void Acceptor::Stop(){
		acceptor_->close();
	}

	void Acceptor::AcceptAsync(Callback callback, std::shared_ptr<Listener> listener) {
		auto& any_scheduler = System::Scheduler::RoundRobin();
		auto any_context = any_scheduler.GetContext();
		auto socket = std::make_unique<Network::Socket>(any_context);
		auto socket_ptr = socket.get();
		acceptor_->async_accept(socket_ptr->raw_socket(), [socket=std::move(socket), callback, weak_listener = std::weak_ptr(listener)](const boost::system::error_code& error) mutable {
			auto listener = weak_listener.lock();
			if (listener == nullptr) {
				return;
			}

			listener->Post([socket=std::move(socket), callback, error](Listener& listener) mutable {
				(listener.*callback)(std::move(socket), error);
			});
		});
	}

	std::string Acceptor::ToString() const {
		if (endpoint_ == nullptr) {
			return "";
		}
		return endpoint_->address().to_string() + ":" + std::to_string(endpoint_->port());
	}

} // namespace Network