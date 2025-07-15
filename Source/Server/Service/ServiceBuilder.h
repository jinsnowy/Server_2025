#pragma once

#include "Server/Service/Service.h"
#include "Core/Network/IPAddress.h"

namespace Server {

	class ServiceBuilder {
	public:
		ServiceBuilder() = default;
		~ServiceBuilder() = default;

		template<typename SessionClass>
		ServiceBuilder& SessionClass() {
			session_factory_.SetSessionClass<SessionClass>();
			return *this;
		}

		ServiceBuilder& OnConnect(std::function<bool(std::shared_ptr<Network::Session>)> on_connect) {
			session_factory_.OnConnect(on_connect);
			return *this;
		}

		ServiceBuilder& UseIp(const std::string& ip) {
			ip_ = ip;
			return *this;
		}

		ServiceBuilder& UsePort(uint16_t port) {
			port_ = port;
			return *this;
		}

		Network::SessionFactory& GetSessionFactory() {
			return session_factory_;
		}

		template<typename ServiceClass = Service>
		std::unique_ptr<ServiceClass> Build() {
			if (ip_.empty() || port_ == 0) {
				throw std::runtime_error("IP and port must be set before building the service.");
			}
			if (session_factory_.is_empty()) {
				throw std::runtime_error("Session class must be set before building the service.");
			}
			auto service = std::make_unique<ServiceClass>();
			service->set_ip_address(Network::IPAddress(ip_, port_));
			service->set_session_factory(session_factory_);
			return service;
		}

	private:
		Network::SessionFactory session_factory_;
		std::string ip_ = "0.0.0.0";
		uint16_t port_ = 0;
	};


}