#include "stdafx.h"
#include "DependencyInjection.h"

namespace System {
	std::unordered_map<size_t, std::unique_ptr<DependencyInjection::ServiceFactoryInterface>> DependencyInjection::service_factories_ = {};

	thread_local std::shared_ptr<ThreadLocalServiceInterface>  DependencyInjection::ThreadLocalServiceFactory::instance = {};
}