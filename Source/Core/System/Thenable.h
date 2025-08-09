#pragma once

#include "Core/System/Detail/AsyncResult.h"
#include "Core/System/FutureState.h"

namespace System {
namespace Detail {
	template<typename T>
	class FutureController;

	template<typename F, typename R, typename _R>
	class WhenResult {
	public:
		WhenResult(F&& func, const std::shared_ptr<FutureState<_R>>& thenable_state, const void* signature)
			:
			func_(std::forward<F>(func)),
			thenable_state_(thenable_state),
			signature_(signature) {
		}

		void operator()(R&& result);

	private:
		F func_;
		std::shared_ptr<FutureState<_R>> thenable_state_;
		const void* signature_ = nullptr;
	};

	struct WhenResultFactory {
		template<typename F, typename R, typename _R>
		static inline WhenResult<F, R, _R> Create(const Thenable<_R>& thenable, F&& func /*Mapping Function R -> _R*/);
	};

	template<typename F, typename R, typename _R>
	class WhenResultAndPost {
	public:
		WhenResultAndPost(F&& func, const std::shared_ptr<FutureState<_R>>& thenable_state, const void* signature)
			:
			func_(std::forward<F>(func)),
			thenable_state_(thenable_state),
			signature_(signature) {
		}

		void operator()(R&& result);

	private:
		F func_;
		std::shared_ptr<FutureState<_R>> thenable_state_;
		const void* signature_ = nullptr;
	};

	template<typename A, typename _R>
	class WhenResultAndPostBody {
	public:
		template<typename F>
		WhenResultAndPostBody(F&& func, const std::shared_ptr<FutureState<_R>>& thenable_state, const void* signature)
			:
			func_(std::forward<F>(func)),
			thenable_state_(thenable_state),
			signature_(signature) {
		}

		void operator()(A& actor);

	private:
		Function<_R(A&)> func_;
		std::shared_ptr<FutureState<_R>> thenable_state_;
		const void* signature_ = nullptr;
	};
		
	struct WhenResultAndPostFactory {
		template<typename F, typename R, typename _R>
		static inline WhenResultAndPost<F, R, _R> Create(const Thenable<_R>& thenable, F&& func);
	};

	template<typename R>
	class Thenable {
	public:
		Thenable(const std::shared_ptr<FutureBase>& parent, const void* signature)
			:
			parent_(parent),
			thenable_state_(std::make_shared<FutureState<R>>()),
			signature_(signature) {
		}

		Thenable& Catch(std::function<void(const std::exception&)> on_exception) {
			parent_->SetExceptionCallback(std::move(on_exception));
			return *this;
		}

		FutureController<R> GetController(const void* signature);

		Thenable(const Thenable&) = delete;
		Thenable& operator=(const Thenable&) = delete;

		Thenable(Thenable&& rhs) noexcept 
			:
			parent_(std::move(rhs.parent_)),
			thenable_state_(std::move(rhs.thenable_state_)),
			signature_(rhs.signature_) {
			rhs.signature_ = nullptr;
		}

		Thenable& operator=(Thenable&& rhs) noexcept {
			if (this != &rhs) {
				parent_ = std::move(rhs.parent_);
				thenable_state_ = std::move(rhs.thenable_state_);
				signature_ = rhs.signature_;
				rhs.signature_ = nullptr;
			}
			return *this;
		}

	private:
		friend struct WhenResultFactory;
		friend struct WhenResultAndPostFactory;

		std::shared_ptr<FutureBase> parent_;
		std::shared_ptr<FutureState<R>> thenable_state_;
		const void* signature_ = nullptr;
	};

	template<typename F, typename R, typename _R>
	inline WhenResult<F, R, _R> WhenResultFactory::Create(const Thenable<_R>& thenable, F&& func) {
		return WhenResult<F, R, _R>(std::forward<F>(func), thenable.thenable_state_, thenable.signature_);
	}

	template<typename F, typename R, typename _R>
	inline WhenResultAndPost<F, R, _R> WhenResultAndPostFactory::Create(const Thenable<_R>& thenable, F&& func) {
		return WhenResultAndPost<F, R, _R>(std::forward<F>(func), thenable.thenable_state_, thenable.signature_);
	}
} // namespace Detail
}
