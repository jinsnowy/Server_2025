#include "stdafx.h"
#include "Authenticator.h"

#include "Core/Http/HttpRequest.h"
#include "Core/Json/Json.h"

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

	std::optional<Model::AccountTokenInfo> Authenticator::ConsumeToken(const std::string& access_token) {
		Http::HttpRequest request(server_url_);
		request.Uri("/auth/validate_token")
			.Method(Http::HttpMethod::kPost)
			.ContentType(Http::HttpContentType::kApplicationJson)
			.Accept(Http::HttpContentType::kApplicationJson)
			.AddHeader("Authorization", access_token);

		Http::HttpResponse response = request.Send();
		if (response.status_code() != Http::HttpStatusCode::kOk) {
			return std::nullopt;
		}

		auto json_doc = Json::JDocument::TryParse(response.body());
		if (!json_doc.has_value()) {
			LOG_ERROR("Failed to parse JSON response: {}", response.body());
			return std::nullopt;
		}

		auto user_id_opt = json_doc->GetValue<std::string>("user_id");
		if (!user_id_opt.has_value()) {
			LOG_ERROR("Missing user_id in response: {}", response.body());
			return std::nullopt;
		}
		auto user_name_opt = json_doc->GetValue<std::string>("username");
		if (!user_name_opt.has_value()) {
			LOG_ERROR("Missing username in response: {}", response.body());
			return std::nullopt;
		}

		return Model::AccountTokenInfo(user_id_opt.value(), user_name_opt.value());
	}

}
