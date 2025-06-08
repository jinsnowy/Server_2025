#include "stdafx.h"
#include "HttpRequest.h"
#include "Core/ThirdParty/Curl.h"

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

	HttpResponse HttpRequest::Send() {
		
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
			return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to initialize cURL handle");
		}

		// generate uri
		if (queries_.has_value()) {
			bool first_query = true;
			for (const auto& [query_key, query_value] : *queries_) {
				if (first_query) {
					first_query = false;
					base_uri_ << '?';
				}
				else {
					base_uri_ << '&';
				}

				char* escaped_query_value = curl_easy_escape(curl_handle, query_value.c_str(), static_cast<int32_t>(query_value.size()));
				if (!escaped_query_value) {
					return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to escape query value: " + query_value);
				}

				base_uri_ << query_key;
				base_uri_ << '=';
				base_uri_ << escaped_query_value;
				curl_free(escaped_query_value);
			}
		}

		// Uri
		const std::string uri = base_uri_.str();
		CURLcode code = curl_easy_setopt(curl_handle, CURLOPT_URL, uri.c_str());
		if (code != CURLE_OK) {
			return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to set URL");
		}

		// Method
		switch (method_) {
			case HttpMethod::kGet:
				curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
				break;
			case HttpMethod::kPost:
				curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
				break;
			case HttpMethod::kPut:
				curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "PUT");
				break;
			case HttpMethod::kDelete:
				curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
				break;
			default:
				return HttpResponse::Error(HttpStatusCode::kBadRequest, "Unsupported HTTP method");
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

		if (!headers_.empty()) {
			for (const auto& [key, value] : headers_) {
				curl_header_guard.Append((key + ": " + value).c_str());
			}
			code = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_header_guard.headers);
			if (code != CURLE_OK) {
				return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to set headers");
			}
		}

		// Body
		if (!body_.empty()) {
			if (method_ == HttpMethod::kGet) {
				return HttpResponse::Error(HttpStatusCode::kBadRequest, "GET requests cannot have a body");
			}
			code = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body_.c_str());
			if (code != CURLE_OK) {
				return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to set POSTFIELDS");
			}
			code = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, body_.size());
			if (code != CURLE_OK) {
				return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to set POSTFIELDSIZE");
			}
		}

		// Set up response handling
		HttpResponse response;
		code = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, std::string* data) {
			size_t total_size = size * nmemb;
			data->append(ptr, total_size);
			return total_size;
		});
		if (code != CURLE_OK) {
			return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to set write function");
		}
		code = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, response.mutable_body());
		if (code != CURLE_OK) {
			return HttpResponse::Error(HttpStatusCode::kInternalServerError, "Failed to set write data");
		}

		code = curl_easy_perform(curl_handle);
		if (code != CURLE_OK) {
			return HttpResponse::Error(HttpStatusCode::kInternalServerError, FORMAT("cURL request failed: {}", curl_easy_strerror(code)));
		}

		int32_t http_status_code = 0;
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_status_code);
		response.set_status_code(static_cast<HttpStatusCode>(http_status_code));
		if (http_status_code != 200) {
			response.set_error_message(FORMAT("HTTP request failed with status code: {}", http_status_code));
		}

		return response;
	}
}
