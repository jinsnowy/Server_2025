#pragma once

namespace System {
	class Context;
	class Callable;
	class Channel {
	public:
		static Channel RoundRobin();

		Channel();
		Channel(const std::shared_ptr<Context>& context) 
			: 
			context_(context) {
		}

		~Channel() = default;

		Channel(const Channel&) = default;
		Channel& operator=(const Channel&) = default;

		bool IsSynchronized() const;

		void Post(std::function<void()> func) const;
		void Post(std::unique_ptr<Callable> callable) const;
		void Post(Callable* callable) const;

		std::shared_ptr<Context>& GetContext() { return context_; }
		const std::shared_ptr<Context>& GetContext() const { return context_; }

	private:
		std::shared_ptr<Context> context_;
	};
} // namespace System


