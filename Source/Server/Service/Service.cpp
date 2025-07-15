#include "stdafx.h"
#include "Service.h"

namespace Server {
	void Service::Start()
	{
		auto context = System::Context::Current();
		listener_ = std::make_shared<Network::Listener>(context->shared_from_this(), session_factory_);
		listener_->Bind(ipaddress_.Ip(), ipaddress_.Port());
		listener_->Listen();
	}

}
