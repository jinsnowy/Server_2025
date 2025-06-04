#pragma once

namespace Http {
	enum class HttpMethod : int8_t {
		kGet = 0,
		kPost = 1,
		kPut = 2,
		kDelete = 3,
	};

	static constexpr const char* MethodToString(HttpMethod method) {
		switch (method) {
			case HttpMethod::kGet: return "GET";
			case HttpMethod::kPost: return "POST";
			case HttpMethod::kPut: return "PUT";
			case HttpMethod::kDelete: return "DELETE";
			default: return "UNKNOWN";
		}
	}

	enum class HttpContentType : int8_t {
		kTextPlain = 0,
		kApplicationJson = 1,
		kApplicationXml = 2,
		kMultipartFormData = 3,
		kApplicationOctetStream = 4,
	};

	static constexpr const char* ContentTypeToString(HttpContentType contentType) {
		switch (contentType) {
			case HttpContentType::kTextPlain: return "text/plain";
			case HttpContentType::kApplicationJson: return "application/json";
			case HttpContentType::kApplicationXml: return "application/xml";
			case HttpContentType::kMultipartFormData: return "multipart/form-data";
			case HttpContentType::kApplicationOctetStream: return "application/octet-stream";
			default: return "unknown";
		}
	}

	enum class HttpStatusCode : int32_t {
		kOk = 200,
		kCreated = 201,
		kAccepted = 202,
		kNoContent = 204,
		kMultipleChoices = 300,
		kMovedPermanently = 301,
		kFound = 302,
		kSeeOther = 303,
		kNotModified = 304,
		kTemporaryRedirect = 307,
		kPermanentRedirect = 308,
		kBadRequest = 400,
		kUnauthorized = 401,
		kForbidden = 403,
		kNotFound = 404,
		kMethodNotAllowed = 405,
		kNotAcceptable = 406,
		kProxyAuthenticationRequired = 407,
		kRequestTimeout = 408,
		kConflict = 409,
		kInternalServerError = 500,
		kNotImplemented = 501,
		kBadGateway = 502,
		kServiceUnavailable = 503,
	};

	std::optional<HttpStatusCode> IntegerToHttpStatusCode(int32_t status_code);
} // namespace Http