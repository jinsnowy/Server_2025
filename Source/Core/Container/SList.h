#pragma once

namespace System {

	namespace Detail {
		class ISList {
		public:
			virtual ~ISList() = default;

			virtual void* Request();
			virtual void* Alloc() = 0;
			virtual void Enqueue(void* buffer) = 0;
		};

		std::unique_ptr<ISList> CreateSListImpl(size_t allocation_size, void(*destructor)(void*));

		template<typename T>
		struct DefaultSListItemDestructor {
			void operator()(T* item) {
				if constexpr (!std::is_trivially_default_constructible_v<T>) {
					item->~T();
				}
			}
		};

		template<typename T>
		static std::unique_ptr<ISList> CreateSList() {
			return CreateSListImpl(sizeof(T), +[](void* ptr) { DefaultSListItemDestructor<T>{}(static_cast<T*>(ptr));  });
		}
	}

	template<typename T>
	class SList {
	public:
		SList()
			:
			impl_(Detail::CreateSList<T>()) {
		}

		bool TryPop(T& out_item) {
			T* item = static_cast<T*>(impl_->Request());
			if (item == nullptr) {
				return false;
			}
			if constexpr (std::is_move_assignable_v<T>) {
				out_item = std::move(*item);
			}
			else {
				out_item = *item;
			}
			count_.fetch_sub(1, std::memory_order_release);
			return true;
		}

		void Push(T&& data) {
			void* buffer = impl_->Alloc();
			new (buffer)T(std::move(data));
			impl_->Enqueue(buffer);
			count_.fetch_add(1, std::memory_order_release);
		}

		void Push(const T& data) {
			void* buffer = impl_->Alloc();
			new (buffer)T(data);
			impl_->Enqueue(buffer);
			count_.fetch_add(1, std::memory_order_release);
		}

		size_t Count() const {
			return count_.load(std::memory_order_acquire);
		}

	private:
		std::atomic<int64_t> count_;
		std::unique_ptr<Detail::ISList> impl_;
	};

} // namespace System