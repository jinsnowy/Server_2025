#include "stdafx.h"
#include "HttpRequest.h"
#include "Core/ThirdParty/Curl.h"
#include "Core/System/Scheduler.h"

namespace Http {
	namespace Detail {
		HttpModule::HttpModule() {
			curl_global_init(CURL_GLOBAL_DEFAULT);
			LOG_INFO("HttpModule initialized with cURL");
		}

		HttpModule::~HttpModule() {
			curl_global_cleanup();
		}

		const HttpModule g_Module = {}; // Ensure the module is initialized at startup
	}

	HttpRequest::HttpRequest(const HttpRequest& rhs)
		:
		base_uri_(rhs.base_uri_.str()),
		method_(rhs.method_),
		queries_(rhs.queries_ ? std::make_optional(*rhs.queries_) : std::nullopt),
		headers_(rhs.headers_),
		body_(rhs.body_) {
	}

	HttpRequest& HttpRequest::operator=(const HttpRequest& rhs) {
		if (this != &rhs) {
			base_uri_.str(rhs.base_uri_.str());
			method_ = rhs.method_;
			queries_ = rhs.queries_ ? std::make_optional(*rhs.queries_) : std::nullopt;
			headers_ = rhs.headers_;
			body_ = rhs.body_;
		}
		return *this;
	}

	HttpResponse HttpRequest::Send() {
		HttpResponse response(*this);
		SendInternal(response.mutable_request(), response);
		return response;
	}

	System::Future<HttpResponse> HttpRequest::SendAsync() {
		return System::Scheduler::RoundRobin().Async([request = *this]() mutable {
			HttpResponse response(request);
			SendInternal(response.mutable_request(), response);
			return response;
		});
	}

	void HttpRequest::SendInternal(HttpRequest& request, HttpResponse& response) {
		struct CurlHandleGuard {
			CURL* handle;
			CurlHandleGuard() : handle(curl_easy_init()) {}
			~CurlHandleGuard() {
				if (handle) {
					curl_easy_cleanup(handle);
				}
			}
		} curl_handle_guard;

		CURL* curl_handle = curl_handle_guard.handle;
		if (!curl_handle) {
			response.SetError(HttpStatusCode::kInternalServerError, 0, "Failed to initialize cURL handle");
			return;
		}

		// generate uri
		auto& queries = request.queries_;
		auto& base_uri = request.base_uri_;
		if (queries.has_value()) {
			bool first_query = true;
			for (const auto& [query_key, query_value] : *queries) {
				if (first_query) {
					first_query = false;
					base_uri << '?';
				}
				else {
					base_uri << '&';
				}

				char* escaped_query_value = curl_easy_escape(curl_handle, query_value.c_str(), static_cast<int32_t>(query_value.size()));
				if (!escaped_query_value) {
					response.SetError(HttpStatusCode::kInternalServerError, 0, "Failed to escape query value: " + query_value);
					return;
				}

				base_uri << query_key;
				base_uri << '=';
				base_uri << escaped_query_value;
				curl_free(escaped_query_value);
			}
		}

		// Uri
		const std::string uri = base_uri.str();
		CURLcode code = curl_easy_setopt(curl_handle, CURLOPT_URL, uri.c_str());
		if (code != CURLE_OK) {
			response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set URL");
			return;
		}

		// Method
		const auto& method = request.method_;
		switch (method) {
			case HttpMethod::kGet:
				if ( code = curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L); code != CURLE_OK) {
					response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set HTTP GET method");
					return;
				}
				break;
			case HttpMethod::kPost:
				if ( code = curl_easy_setopt(curl_handle, CURLOPT_POST, 1L); code != CURLE_OK) {
					response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set HTTP POST method");
					return;
				}
				break;
			case HttpMethod::kPut:
				if ( code = curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "PUT"); code != CURLE_OK) {
					response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set HTTP PUT method");
					return;
				}
				break;
			case HttpMethod::kDelete:
				if ( code = curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE"); code != CURLE_OK) {
					response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set HTTP DELETE method");
					return;
				}
				break;
			default:
				response.SetError(HttpStatusCode::kBadRequest, 0, "Unsupported HTTP method");
				return;
		}

		// Headers
		struct CurlHeaderGuard {
			struct curl_slist* headers;
			CurlHeaderGuard() : headers(nullptr) {}

			void Append(const std::string& header) {
				headers = curl_slist_append(headers, header.c_str());
			}

			~CurlHeaderGuard() {
				if (headers) {
					curl_slist_free_all(headers);
				}
			}
		} curl_header_guard;

		const auto& headers = request.headers_;
		if (!headers.empty()) {
			for (const auto& [key, value] : headers) {
				curl_header_guard.Append((key + ": " + value).c_str());
			}
			code = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_header_guard.headers);
			if (code != CURLE_OK) {
				response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set headers");
				return;
			}
		}

		// Body
		const auto& body = request.body_;
		if (!body.empty()) {
			if (method == HttpMethod::kGet) {
				response.SetError(HttpStatusCode::kBadRequest, code, "GET requests cannot have a body");
				return;
			}
			code = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body.c_str());
			if (code != CURLE_OK) {
				response.SetError(HttpStatusCode::kBadRequest, code, "Failed to set POSTFIELDS");
				return;
			}
			code = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, body.size());
			if (code != CURLE_OK) {
				response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set POSTFIELDSIZE");
				return;
			}
		}

		// timeout
		code = curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, request.timeout_);
		if (code != CURLE_OK) {
			response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set timeout");
			return;
		}

		// Set up response handling
		code = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, std::string* data) {
			size_t total_size = size * nmemb;
			data->append(ptr, total_size);
			return total_size;
		});
		if (code != CURLE_OK) {
			response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set write function");
			return;
		}
		code = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, response.mutable_body());
		if (code != CURLE_OK) {
			response.SetError(HttpStatusCode::kInternalServerError, code, "Failed to set write data");
			return;
		}

		code = curl_easy_perform(curl_handle);
		if (code != CURLE_OK) {
			response.SetError(HttpStatusCode::kInternalServerError, code, FORMAT("cURL request failed: {}", curl_easy_strerror(code)));
			return;
		}

		int32_t http_status_code = 0;
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_status_code);
		response.set_status_code(static_cast<HttpStatusCode>(http_status_code));
		if (http_status_code != 200) {
			response.set_error_message(FORMAT("HTTP request failed with status code: {}", http_status_code));
		}

		return;
	}
}
