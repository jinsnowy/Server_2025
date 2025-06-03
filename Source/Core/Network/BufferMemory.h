#pragma once

namespace Network {
	class BufferOwner {
	public:
		virtual ~BufferOwner() = default;

		virtual void OnBufferReleased(std::unique_ptr<char[]> buffer, size_t buffer_size) = 0;
	};

	class BufferMemory : public std::enable_shared_from_this<BufferMemory> {
	public:
		BufferMemory(std::unique_ptr<char[]> buffer, size_t buffer_size_in, std::weak_ptr<BufferOwner> owner)
			: 
			buffer(std::move(buffer)),
			buffer_size(buffer_size_in),
			owner(std::move(owner)) {
		}

		~BufferMemory() {
			if (auto owner_ptr = owner.lock()) {
				owner_ptr->OnBufferReleased(std::move(buffer), buffer_size);
			}
		}

		void Clear() {
			std::fill(buffer.get(), buffer.get() + buffer_size, 0); // Clear the buffer
		}

		char* GetBufferPtr() {
			return buffer.get();
		}

		const char* GetBufferPtr() const {
			return buffer.get();
		}

		size_t GetBufferSize() const {
			return buffer_size;
		}

	private:
		std::unique_ptr<char[]> buffer;
		size_t buffer_size;
		std::weak_ptr<BufferOwner> owner;
	};
}

