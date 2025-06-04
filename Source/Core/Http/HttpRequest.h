#pragma once

#include "Core/Http/HttpEnums.h"
#include "Core/Http/HttpResponse.h"

namespace Http {
	class HttpRequest {
	public:
		HttpRequest(const std::string& base_uri)
			:
			base_uri_() {
			base_uri_ << base_uri;
			if (!base_uri.empty() && base_uri.back() != '/') {
				base_uri_ << '/';
			}
		}

		HttpRequest& Uri(const std::string& uri) {
			if (uri.empty() == false && uri.front() == '/') {
				base_uri_ << uri.substr(1); // Remove leading slash if present
			}
			else {
				base_uri_ << uri;
			}
			return *this;
		}
		
		HttpRequest& Method(HttpMethod method) {
			method_ = method;
			return *this;
		}

		HttpRequest& ContentType(HttpContentType content_type) {
			headers_["Content-Type"] = ContentTypeToString(content_type);
			return *this;
		}

		HttpRequest& Accept(HttpContentType content_type) {
			headers_["Accept"] = ContentTypeToString(content_type);
			return *this;
		}

		HttpRequest& AddQuery(const std::string& key, const std::string& value) {
			if (!queries_) {
				queries_.emplace(std::map<std::string, std::string>{});
			}
			(*queries_)[key] = value;
		}

		HttpRequest& AddHeader(const std::string& key, const std::string& value) {
			headers_[key] = value;
			return *this;
		}

		HttpRequest& Body(const std::string& body) {
			headers_["Content-Length"] = std::to_string(body.size());
			body_ = body;
			return *this;
		}

		HttpRequest& Body(std::string&& body) {
			headers_["Content-Length"] = std::to_string(body.size());
			body_ = std::move(body);
			return *this;
		}

		HttpResponse Send();

		std::string GetUri() const {
			return base_uri_.str();
		}

	private:
		std::stringstream base_uri_;
		HttpMethod method_ = HttpMethod::kGet; // Default method
		std::optional<std::map<std::string, std::string>> queries_;
		std::map<std::string, std::string> headers_;
		std::string body_;
	};

#pragma init_seg(user)
	namespace Detail {
		class HttpModule {
		public:
			HttpModule();
			~HttpModule();
		};

		extern const HttpModule g_Module;
	} // namespace Detail
} // namespace Http


