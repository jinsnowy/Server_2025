#include "stdafx.h"
#include "IPAddress.h"

#include "Core/ThirdParty/BoostAsio.h"
#include "Core/System/String.h"
#include "Core/System/Context.h"

#include <boost/lexical_cast.hpp>

namespace Network {
	IPAddress::IPAddress(const uint8_t ipv4[4], uint16_t port)
		:
		ip_(std::format("%d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3])),
		port_(port)
	{
	}

	std::optional<IPAddress> IPAddress::Resolve(const std::string& ip, uint16_t port) {
		std::string ip_str = System::String::Trim(ip);
		std::smatch match;
		std::regex ipv4_regex(R"((\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3}))");
		// Check if the input is a valid IPv4 address
		if (std::regex_match(ip_str, match, ipv4_regex))
		{
			try {
				uint8_t ipv4[4] = { 0 };
				ipv4[0] = static_cast<uint8_t>(std::stoi(match[1].str()));
				ipv4[1] = static_cast<uint8_t>(std::stoi(match[2].str()));
				ipv4[2] = static_cast<uint8_t>(std::stoi(match[3].str()));
				ipv4[3] = static_cast<uint8_t>(std::stoi(match[4].str()));
				return IPAddress(ipv4, port);
			}
			catch (const std::exception&) {
				LOG_ERROR("Invalid IPv4 address: {}", ip_str);
				return std::nullopt;
			}
		}

		try {
			boost::asio::io_context& io_context = System::Context::Current()->io_context();
			boost::asio::ip::tcp::resolver resolver(io_context);
			boost::asio::ip::tcp::resolver::results_type results = resolver.resolve(ip_str, boost::lexical_cast<std::string>(port));

			if (results.empty()) {
				LOG_ERROR("No results found for IP address: {}", ip_str);
				return std::nullopt;
			}

			const auto& entry = *results.begin();
			auto endpoint = entry.endpoint();
			std::string entry_ip = endpoint.address().to_string();
			uint16_t entry_port = endpoint.port();
			return IPAddress(entry_ip, entry_port);
		}
		catch (const boost::system::system_error& e) {
			LOG_ERROR("Failed to resolve IP address: {}, error: {}", ip_str, e.what());
			return std::nullopt;
		}
	}
}

