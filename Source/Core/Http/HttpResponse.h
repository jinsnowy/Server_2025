#pragma once

#include "Core/Http/HttpEnums.h"

namespace Http {
	class HttpResponse {
	public:
		HttpResponse() = default;

		static HttpResponse Error(HttpStatusCode status_code, const std::string& error_message) {
			HttpResponse response;
			response.status_code_ = status_code;
			response.error_message_ = error_message;
			return response;
		}

		HttpResponse(HttpStatusCode status_code, const std::string& body)
			: status_code_(status_code), body_(body) {
		}

		HttpStatusCode status_code() const {
			return status_code_;
		}

		void set_status_code(HttpStatusCode status_code) {
			status_code_ = status_code;
		}

		void set_error_message(const std::string& error_message) {
			error_message_ = error_message;
		}

		const std::string& body() const {
			return body_;
		}

		std::string* mutable_body() {
			return &body_;
		}

	private:
		HttpStatusCode status_code_ = HttpStatusCode::kOk; // Default to 200 OK
		std::optional<std::string> error_message_;
		std::string body_;
	};
}
