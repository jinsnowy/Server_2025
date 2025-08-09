#pragma once

#include "Core/Container/MPSC.h"
#include "Core/System/Context.h"
#include "Core/System/Function.h"

namespace System {
	namespace Detail {
		class IMessage;
	} // namespace Detail

	class Dispatcher;
	class Message;
	class Channel {
	public:
		~Channel();

		static Channel Acquire();
		static Channel Empty();

		Channel(const Channel&);
		Channel& operator=(const Channel&);

		Channel(Channel&&)noexcept;
		Channel& operator=(Channel&&)noexcept;

		bool IsSynchronized() const { return state_->current_context_ == Context::Current(); }
		size_t Dispatched(size_t count) const;

		void Post(Message* message);

		void Push(Detail::IMessage* message);
		bool TryPop(Detail::IMessage*& message);

		bool IsEmpty() const;
		size_t GetRefCount() const;

	private:
		friend class ExecutionContext;
		friend class Dispatcher;

		struct State {
			Context* current_context_ = nullptr;
			Container::MPSCQueue<Detail::IMessage*> message_queue_;
			State();
			~State();
		};

		struct EmptyTag {
		};

		Channel();
		Channel(EmptyTag);

		std::shared_ptr<State> state_;

		void BeginContext(Context* context) {
			state_->current_context_ = context;
		}

		void EndContext() {
			state_->current_context_ = nullptr;
		}
	};

} // namespace System


