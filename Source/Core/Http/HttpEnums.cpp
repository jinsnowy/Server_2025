#include "stdafx.h"
#include "HttpEnums.h"

namespace Http {
	std::optional<HttpStatusCode> IntegerToHttpStatusCode(int32_t status_code) {
		switch (status_code) {
		case 200: return HttpStatusCode::kOk;
		case 201: return HttpStatusCode::kCreated;
		case 202: return HttpStatusCode::kAccepted;
		case 204: return HttpStatusCode::kNoContent;
		case 300: return HttpStatusCode::kMultipleChoices;
		case 301: return HttpStatusCode::kMovedPermanently;
		case 302: return HttpStatusCode::kFound;
		case 303: return HttpStatusCode::kSeeOther;
		case 304: return HttpStatusCode::kNotModified;
		case 307: return HttpStatusCode::kTemporaryRedirect;
		case 308: return HttpStatusCode::kPermanentRedirect;
		case 400: return HttpStatusCode::kBadRequest;
		case 401: return HttpStatusCode::kUnauthorized;
		case 403: return HttpStatusCode::kForbidden;
		case 404: return HttpStatusCode::kNotFound;
		case 405: return HttpStatusCode::kMethodNotAllowed;
		case 406: return HttpStatusCode::kNotAcceptable;
		case 407: return HttpStatusCode::kProxyAuthenticationRequired;
		case 408: return HttpStatusCode::kRequestTimeout;
		case 409: return HttpStatusCode::kConflict;
		case 500: return HttpStatusCode::kInternalServerError;
		case 501: return HttpStatusCode::kNotImplemented;
		case 502: return HttpStatusCode::kBadGateway;
		case 503: return HttpStatusCode::kServiceUnavailable;
		default:
			return std::nullopt; // Invalid status code
		}
	}
}
