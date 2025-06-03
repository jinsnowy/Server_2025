#include "stdafx.h"
#include "Resolver.h"

#include "Core/System/Context.h"
#include "Core/Network/Connection.h"

namespace Network {

	Resolver::Resolver(const std::shared_ptr<System::Context>& context)
		:
		resolver_(std::make_unique<boost::asio::ip::tcp::resolver>(context->io_context()))
	{
	}

	Resolver::~Resolver() = default;

	void Resolver::Resolve(const std::string& ip, const uint16_t& port, Callback callback, std::shared_ptr<Connection> conn, std::shared_ptr<Session> session) {
		resolver_->async_resolve(ip, std::to_string(port), [weak_conn=std::weak_ptr(conn), callback, weak_session=std::weak_ptr(session)](const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results) {
			auto conn = weak_conn.lock();
			if (conn == nullptr) {
				return;
			}

			auto session = weak_session.lock();
			if (session == nullptr) {
				return;
			}

			Ctrl(*conn).Post([callback, error, results = std::move(results), session](Connection& conn) mutable {
				(conn.*callback)(error, results, session);
			});
		});
	}

}