#pragma once

namespace Container {
	template<typename T, int32_t BufferCount>
	class CircularBuffer {
	public:
		CircularBuffer() = default;
		~CircularBuffer() = default;

		void Push(const T& input) {
			buffers_[head_] = input;
			head_ = (head_ + 1) % BufferCount;
			if (head_ == tail_) {
				tail_ = (tail_ + 1) % BufferCount; // Overwrite the oldest element
			}
		}

		bool TryPop(T& output) {
			if (tail_ == head_) {
				return false; // Buffer is empty
			}
			if constexpr (std::is_move_assignable_v<T>) {
				output = std::move(buffers_[tail_]);
			}else{
				output = buffers_[tail_]; // Copy if T is not move assignable
			}
			tail_ = (tail_ + 1) % BufferCount;
			return true;
		}

		T* Front() {
			if (tail_ == head_) {
				return nullptr; // Buffer is empty
			}
			return &buffers_[tail_];
		}

		T* Back() {
			if (tail_ == head_) {
				return nullptr; // Buffer is empty
			}
			return &buffers_[(head_ - 1 + BufferCount) % BufferCount];
		}

		bool IsEmpty() const {
			return tail_ == head_;
		}

		int32_t Size() const {
			return (head_ - tail_ + BufferCount) % BufferCount;
		}

		void Clear() {
			head_ = tail_;
			buffers_.fill(T()); // Reset all elements to default value
		}

	private:
		int32_t head_ = 0;
		int32_t tail_ = 0;
		std::array<T, BufferCount> buffers_;
	};
}


