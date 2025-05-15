#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace System {
	class Context; 
} // namespace System {

namespace Network {
	class Connection;
	class Session;
	class Resolver {
	public:
		using Callback = void (Connection::*)(const boost::system::error_code&, boost::asio::ip::tcp::resolver::results_type, std::shared_ptr<Session>);

		Resolver(const std::shared_ptr<System::Context>& context);
		~Resolver();

		void Resolve(const std::string& ip, const uint16_t& port, Callback callback, std::shared_ptr<Connection> conn, std::shared_ptr<Session> session);

	private:
		std::unique_ptr<boost::asio::ip::tcp::resolver> resolver_;
	};
}

