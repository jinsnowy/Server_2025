#include "stdafx.h"
#include "BufferPool.h"
#include "Core/Network/BufferMemory.h"
#include "Core/System/ThreadLocalStorage.h"
#include "Core/Container/MPSC.h"

namespace Network {
	template<typename T, size_t ChunkSize>
	class BufferPool : public System::ThreadLocalStorageShared<T>, public BufferOwner {
	public:
		static constexpr size_t kAllocationInitUnit = 128; // Default element size for the buffer pool
		static constexpr size_t kAllocationOnceUnit = 16; // Default element size for the buffer pool
		static constexpr size_t kMaxPoolSize = 1024;
		static constexpr size_t kSize = ChunkSize;

		BufferPool()
			:
			buffer_pool_() {
			for (size_t i = 0; i < kAllocationInitUnit; ++i) { // Preallocate 100 buffers
				buffer_pool_.Push(std::make_unique<char[]>(ChunkSize));
			}
			current_size_ = kAllocationInitUnit;
		}

		~BufferPool() {
			std::unique_ptr<char[]> buffer;
			while (buffer_pool_.TryPop(buffer)) {
			}
		}

		void OnBufferReleased(std::unique_ptr<char[]> buffer, size_t) override {
			const int32_t captured_size = current_size_.load(std::memory_order_relaxed);
			if (captured_size >= kMaxPoolSize) {
				return; // Do not push back if the pool is already at max size
			}
			++current_size_;
			buffer_pool_.Push(std::move(buffer));
		}

		std::unique_ptr<char[]> RequestBuffer() {
			std::unique_ptr<char[]> buffer;
			while (!buffer_pool_.TryPop(buffer)) {
				AllocateOnce();
			}
			--current_size_;
			return buffer;
		}

		std::weak_ptr<BufferOwner> GetOwner() {
			return System::ThreadLocalStorageShared<T>::shared_from_this();
		}

	private:
		std::atomic<int32_t> current_size_{0};
		Container::MPSCQueue<std::unique_ptr<char[]>> buffer_pool_;

		void AllocateOnce() {
			for (size_t i = 0; i < kAllocationOnceUnit; ++i) { // Preallocate 100 buffers
				buffer_pool_.Push(std::make_unique<char[]>(ChunkSize));
			}
			current_size_ += kAllocationOnceUnit;
		}
	};

	class SendBufferPool : public BufferPool<SendBufferPool, 4096> {
	public:
	};
	
	class RecvBuferPool : public BufferPool<RecvBuferPool, 4096> {
	public:
	};

	Buffer RequestSendBuffer() {
		auto& pool = SendBufferPool::GetInstance();
		auto buffer = pool.RequestBuffer();
		auto buffer_memory = std::make_shared<BufferMemory>(std::move(buffer), SendBufferPool::kSize, pool.GetOwner());
		return Buffer(buffer_memory);
	}

	Buffer RequestRecvBuffer() {
		auto& pool = RecvBuferPool::GetInstance();
		auto buffer = pool.RequestBuffer();
		auto buffer_memory = std::make_shared<BufferMemory>(std::move(buffer), RecvBuferPool::kSize, pool.GetOwner());
		return Buffer(buffer_memory);
	}

	Buffer RequestBuffer(size_t BufferSize) {
		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(BufferSize);
		return std::make_shared<BufferMemory>(std::move(buffer), BufferSize, std::weak_ptr<Network::BufferOwner>{});
	}
}
