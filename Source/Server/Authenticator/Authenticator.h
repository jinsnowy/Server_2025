#pragma once

#include "Core/System/SingletonActor.h"
#include "Server/Model/Account.h"

namespace Server {
	class Authenticator : public System::SingletonActor<Authenticator> {
	public:
		Authenticator();
	
		void Initialize(const std::string& server_url);

		std::optional<Model::AccountTokenInfo> ConsumeToken(const std::string& access_token);

	private:
		std::string server_url_;
	};
}

