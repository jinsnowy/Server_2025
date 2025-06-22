#pragma once

#include "Core/Http/HttpEnums.h"

namespace Http {
	class HttpRequest;
	class HttpResponse {
	public:
		HttpResponse();
		HttpResponse(const HttpRequest& request);
		~HttpResponse();

		HttpResponse(const HttpResponse&) = delete;
		HttpResponse& operator=(const HttpResponse&) = delete;

		HttpResponse(HttpResponse&&) = default;
		HttpResponse& operator=(HttpResponse&&) = default;

		void SetError(HttpStatusCode status_code, int32_t curl_code, const std::string& error_message) {
			status_code_ = status_code;
			curl_code_ = curl_code;
			error_message_ = error_message;
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

		const HttpRequest& request() {
			if (!request_) {
				return *kInvalidRequest;
			}
			return *request_;
		}

		HttpRequest& mutable_request() {
			if (!request_) {
				request_ = std::make_unique<HttpRequest>();
			}
			return *request_;
		}

		int32_t curl_code() const {
			return curl_code_;
		}

	private:
		static std::unique_ptr<HttpRequest> kInvalidRequest;
		std::unique_ptr<HttpRequest> request_;
		HttpStatusCode status_code_ = HttpStatusCode::kOk; // Default to 200 OK
		std::optional<std::string> error_message_;
		std::string body_;
		int32_t curl_code_ = 0; // CURL error code, 0 means no error
	};
}
