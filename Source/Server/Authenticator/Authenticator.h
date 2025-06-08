#pragma once

#include "Core/System/SingletonActor.h"

namespace Server {
	class Authenticator : public System::SingletonActor<Authenticator> {
	public:
		Authenticator();
	
		void Initialize(const std::string& server_url);

		bool ValidateAccessToken(const std::string& access_token);

	private:
		std::string server_url_;
	};
}

