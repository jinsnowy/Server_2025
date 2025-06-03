#pragma once

// https://github.com/chronoxor/CppBenchmark/blob/master/examples/lockfree/mpsc-queue.hpp

// This is free and unencumbered software released into the public domain.

// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// For more information, please refer to <http://unlicense.org/>

// C++ implementation of Dmitry Vyukov's non-intrusive lock free unbound MPSC queue
// http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue

namespace Container {
	template<typename T>
	class MPSCQueue {
	public:
		MPSCQueue() 
			:
			_cache_padding{}  {
			_head = _tail = new buffer_node_t();
		}

		~MPSCQueue() {
			UnsafeClear();
		}

		void Push(T&& input) {
			buffer_node_t* node = new buffer_node_t(std::move(input));
			buffer_node_t* prev_head = _head.exchange(node, std::memory_order_acq_rel);
			prev_head->next.store(node, std::memory_order_release);
		}

		void Push(const T& input) {
			buffer_node_t* node = new buffer_node_t(input);
			buffer_node_t* prev_head = _head.exchange(node, std::memory_order_acq_rel);
			prev_head->next.store(node, std::memory_order_release);
		}

		bool TryPop(T& output) {
			buffer_node_t* tail = _tail.load(std::memory_order_relaxed);
			buffer_node_t* next = tail->next.load(std::memory_order_acquire);

			if (next == nullptr) {
				return false;
			}

			output = std::move(next->data);
			_tail.store(next, std::memory_order_release);
			delete tail;
		
			return true;
		}

		bool IsEmpty() const {
			return _tail.load(std::memory_order_relaxed)->next.load(std::memory_order_relaxed) == nullptr;
		}

		void UnsafeClear() {
			T output;
			while (TryPop(output)) {
			}
		}

		size_t UnsafeSize() const {
			size_t size = 0;
			buffer_node_t* node = _tail.load();
			while (node->next.load() != nullptr) {
				node = node->next.load();
				size++;
			}
			return size;
		}

	private:
		struct buffer_node_t {
			T                           data;
			std::atomic<buffer_node_t*> next;

			buffer_node_t() : data{}, next(nullptr) {}
			buffer_node_t(T&& data_) : data(std::move(data_)), next{} {}
			buffer_node_t(const T& data_) : data(data_), next{} {}
		};

		static constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;

		std::atomic<buffer_node_t*> _head;
		char 					    _cache_padding[kCacheLineSize - sizeof(std::atomic<buffer_node_t*>)];
		std::atomic<buffer_node_t*> _tail;

		MPSCQueue(const MPSCQueue&) {}
		void operator=(const MPSCQueue&) {}
	};
}

