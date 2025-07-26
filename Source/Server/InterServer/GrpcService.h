#pragma once

#include "Protobuf/Public/GrpcHeader.h"
#include "Core/System/Scheduler.h"

namespace Server 
{
	class GrpcCallException : public std::exception {
	public:
		GrpcCallException(grpc::StatusCode code, const std::string& message)
			: code_(code), message_(message) {
		}

		grpc::StatusCode code() const {
			return code_;
		}
		
		const std::string& message() const {
			return message_;
		}

		const char* what() const noexcept override {
			return message_.c_str();
		}

	private:
		grpc::StatusCode code_;
		std::string message_;
	};
}