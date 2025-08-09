#pragma once

#include "Core/System/Detail/IMessage.h"
#include "Core/System/Function.h"

namespace System {
	class Message : public Detail::IMessage {
	public:
		Message(Function<void()>&& function, const void* signature)
			:
			function_(std::move(function)),
			signature_(signature) {
		}

		void Execute() override {
			function_();
		}

		const void* signature() const override {
			return signature_;
		}

	private:
		Function<void()> function_;
		const void* signature_;
	};

	class MessageFactory {
	public:
		MessageFactory(const void* signature)
			:
			signature_(signature) {
		}

		template<typename F>
		Message* Create(F&& func) {
			return new Message(std::forward<F>(func), signature_);
		}

	private:
		const void* signature_;
	};

} // namespace System

#define BUILD_MESSAGE System::MessageFactory(_ReturnAddress()).Create
