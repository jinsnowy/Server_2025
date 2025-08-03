#pragma once

namespace Network {
	class IPAddress {
	public:
		IPAddress() = default;
		IPAddress(const std::string& ip, uint16_t port)
			: ip_(ip), port_(port) {}

		IPAddress(const uint8_t ipv4[4], uint16_t port);
		
		static std::optional<IPAddress> Resolve(const std::string& ip, uint16_t port);

		const std::string& Ip() const { return ip_; }
		uint16_t Port() const { return port_; }
		
		std::string ToString() const {
			return ip_ + ":" + std::to_string(port_);
		}

		bool IsValid() const {
			return !ip_.empty() && port_ > 0 && port_ <= 65535;
		}

		void Reset() {
			ip_.clear();
			port_ = 0;
		}

	private:
		std::string ip_;
		uint16_t port_ = 0;
	};
}
