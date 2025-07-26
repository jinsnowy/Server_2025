#pragma once

#include "Core/System/FuncTraits.h"

namespace System {
	class Callable {
	public:
		virtual ~Callable() = default;
		virtual void operator()() = 0;
	};

	namespace Detail {
		template<typename F, typename ...Args>
		class Callback : public Callable {
		public:
			Callback(F&& func, Args&&... args)
				: func_(std::forward<F>(func)), args_(std::make_tuple(std::forward<Args>(args)...)) {
				static_assert(std::is_same_v<FuncReturnT<F>, void>, "Callback instance should return void");
			}

			void operator()() override {
				std::apply(func_, std::move(args_));
			}

		private:
			F func_;
			std::tuple<Args...> args_;
		};

		template<typename F>
		class Callback<F> : public Callable {
		public:
			Callback(F&& func)
				: func_(std::forward<F>(func)) {
				static_assert(std::is_same_v<FuncReturnT<F>, void>, "Callback instance should return void");
			}

			void operator()() override {
				func_();
			}
		private:
			F func_;
		};

		struct CallablePtr {
			std::unique_ptr<::System::Callable> callable;

			void operator()() {
				if (callable) {
					(*callable)();
				}
			}
		};
	}
	
	template<typename F, typename ...Args>
	static inline std::unique_ptr<Callable> MakeCallable(F&& func, Args&&... args) {
		return std::make_unique<Detail::Callback<F, Args...>>(std::forward<F>(func), std::forward<Args>(args)...);
	}

	template<typename F>
	static inline std::unique_ptr<Callable> MakeCallable(F&& func) {
		return std::make_unique<Detail::Callback<F>>(std::forward<F>(func));
	}
}