#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace System {
	class Context;
} // namespace System 

namespace Network {
	class Listener;
	class Socket;
	class Acceptor final {
	public:
		using Callback = void (Listener::*)(std::unique_ptr<Socket>, const boost::system::error_code&);

		Acceptor(std::shared_ptr<System::Context> context);
		~Acceptor();

		void Bind (std::string ip, uint16_t port);
		void Listen();
		void Stop();
		void AcceptAsync(Callback callback, std::shared_ptr<Listener> listener);
		std::string ToString() const;

	private:
		std::shared_ptr<System::Context> context_;
		std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
		std::unique_ptr<boost::asio::ip::tcp::endpoint> endpoint_;
	};
}

