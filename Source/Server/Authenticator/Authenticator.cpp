#include "stdafx.h"
#include "Authenticator.h"

#include "Core/Http/HttpRequest.h"

namespace Server {
	Authenticator::Authenticator() {
	}

	void Authenticator::Initialize(const std::string& server_url) {
		server_url_ = server_url;

		Http::HttpRequest ping_request(server_url_);
		ping_request.Uri("/ping")
			.Method(Http::HttpMethod::kGet)
			.Accept(Http::HttpContentType::kApplicationJson);

		Http::HttpResponse response = ping_request.Send();
		if (response.status_code() != Http::HttpStatusCode::kOk) {
			LOG_ERROR("Failed to ping authentication server: {}", response.body());
			return;
		}
	}

	bool Authenticator::ValidateAccessToken(const std::string& access_token) {
		Http::HttpRequest request(server_url_);
		request.Uri("/auth/validate_token")
			.Method(Http::HttpMethod::kPost)
			.ContentType(Http::HttpContentType::kApplicationJson)
			.Accept(Http::HttpContentType::kApplicationJson)
			.AddHeader("Authorization", access_token);

		Http::HttpResponse response = request.Send();
		if (response.status_code() == Http::HttpStatusCode::kOk) {
			return true;
		}

		return false;
	}

}
