#include "stdafx.h"
#include "HttpResponse.h"
#include "HttpRequest.h"

namespace Http {
	std::unique_ptr<HttpRequest> HttpResponse::kInvalidRequest = std::make_unique<HttpRequest>();


	HttpResponse::HttpResponse()
	{
	}

	HttpResponse::HttpResponse(const HttpRequest& request)
		:
		request_(std::make_unique<HttpRequest>(request))
	{
	}

	HttpResponse::~HttpResponse()
	{
	}
}