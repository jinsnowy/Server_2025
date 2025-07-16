#pragma once

#include "Core/System/TypeInfo.h"

namespace System {
	class SingletonServiceInterface;
	class ThreadLocalServiceInterface;
	class TransientServiceInterface;

	class DependencyInjection final {
	public:
		template<typename T>
		static void RegisterService(std::function<std::shared_ptr<T>()> factory) {
			std::function<std::shared_ptr<void> ()> service_factory = [factory]() {
				return std::static_pointer_cast<void>(factory());
			};
			size_t type_id = DependencyInjection::GetTypeId<T>();
			if constexpr (std::is_base_of<SingletonServiceInterface, T>::value) {
				service_factories_[type_id] = std::make_unique<SingletonServiceFactory>(service_factory);
			} else if constexpr (std::is_base_of<ThreadLocalServiceInterface, T>::value) {
				service_factories_[type_id] = std::make_unique<ThreadLocalServiceFactory>(service_factory);
			} else if constexpr (std::is_base_of<TransientServiceInterface, T>::value) {
				service_factories_[type_id] = std::make_unique<TransientServiceFactory>(service_factory);
			} else {
				throw std::runtime_error("Unsupported service type");
			}

			LOG_INFO("Registered service: {} with type_id: {}", DependencyInjection::GetName<T>(), type_id);
		}

		template<typename T>
		static std::shared_ptr<T> Get() {
			size_t type_id = DependencyInjection::GetTypeId<T>();
			auto it = service_factories_.find(type_id);
			if (it == service_factories_.end()) {
				throw std::runtime_error("Service not registered: " + std::string(DependencyInjection::GetName<T>()));
			}
			return std::static_pointer_cast<T>(it->second->Create());
		}

	private:
		template<typename T>
		static constexpr size_t GetTypeId() {
			return System::hashcode<std::remove_cv_t<T>>();
		}

		template<typename T>
		static constexpr std::string_view GetName() {
			return System::type_name_v<std::remove_cv_t<T>>;
		}

		struct ServiceFactoryInterface {
			std::function<std::shared_ptr<void>()> factory;
			ServiceFactoryInterface(std::function<std::shared_ptr<void>()> factory) : factory(std::move(factory)) {}
			virtual ~ServiceFactoryInterface() = default;
			virtual std::shared_ptr<void> Create() = 0;
		};

		struct SingletonServiceFactory : ServiceFactoryInterface {
			std::once_flag initialized_flag;
			std::shared_ptr<SingletonServiceInterface> instance;
			SingletonServiceFactory(std::function<std::shared_ptr<void>()> factory)
				: ServiceFactoryInterface(std::move(factory)), instance(nullptr) {
			}
			std::shared_ptr<void>  Create() override {
				std::call_once(initialized_flag, [this]() {
					instance = std::static_pointer_cast<SingletonServiceInterface>(factory());
				});
				return instance;
			}
		};

		struct ThreadLocalServiceFactory : ServiceFactoryInterface {
			static thread_local std::shared_ptr<ThreadLocalServiceInterface> instance;
			ThreadLocalServiceFactory(std::function<std::shared_ptr<void>()> factory)
				: ServiceFactoryInterface(std::move(factory)) {
			}
			std::shared_ptr<void>  Create() override {
				if (!instance) {
					instance = std::static_pointer_cast<ThreadLocalServiceInterface>(factory());
				}
				return instance;
			}
		};

		struct TransientServiceFactory : ServiceFactoryInterface {
			TransientServiceFactory(std::function<std::shared_ptr<void>()> factory)
				: ServiceFactoryInterface(std::move(factory)) {
			}
			std::shared_ptr<void>  Create() override {
				return factory();
			}
		};

		static std::unordered_map<size_t, std::unique_ptr<ServiceFactoryInterface>> service_factories_;
	};

}

