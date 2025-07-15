#pragma once

#include "Core/Network/SessionFactory.h"
#include "Core/Network/IPAddress.h"
#include "Core/Network/Listener.h"

namespace Server {
	class Service {
	public:
		Service() = default;
		Service(const Network::IPAddress& ipaddress, Network::SessionFactory session_factory)
			:
			ipaddress_(ipaddress),
			session_factory_(std::move(session_factory)) {
		}
		virtual ~Service() = default;

		virtual void Start();

		void set_ip_address(const Network::IPAddress& ipaddress) {
			ipaddress_ = ipaddress;
		}

		void set_session_factory(Network::SessionFactory session_factory) {
			session_factory_ = std::move(session_factory);
		}

	private:
		Network::IPAddress ipaddress_;
		Network::SessionFactory session_factory_;
		std::shared_ptr<Network::Listener> listener_;
	};
}


