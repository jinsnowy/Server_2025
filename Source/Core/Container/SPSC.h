#pragma once
/**
 * Fast single-producer/single-consumer unbounded concurrent queue. Doesn't free memory until destruction but recycles consumed items.
 * Based on http://www.1024cores.net/home/lock-free-algorithms/queues/unbounded-spsc-queue
 */

namespace Container {

	template<typename T>
	class SPSCQueue final {
	public:
		using ElementType = T;

		SPSCQueue(const SPSCQueue&) = delete;
		SPSCQueue& operator=(const SPSCQueue&) = delete;

		SPSCQueue() {
			FNode* node = new FNode();
			tail_.store(node, std::memory_order_relaxed);
			head_ = first_ = tail_copy_ = node;
		}

		~SPSCQueue() {
			FNode* node = first_;
			FNode* local_tail = tail_.load(std::memory_order_relaxed);

			// Delete all nodes which are the sentinel or unoccupied
			bool continued = false;
			do {
				FNode* next = node->next_.load(std::memory_order_relaxed);
				continued = node != local_tail;
				delete node;
				node = next;
			} while (continued);

			// Delete all nodes which are occupied, destroying the element first
			while (node != nullptr) {
				FNode* next = node->next_.load(std::memory_order_relaxed);
				DestructItem(reinterpret_cast<ElementType*>(&node->value_));
				delete node;
				node = next;
			}
		}

		template <typename... ArgTypes>
		void Enqueue(ArgTypes&&... Args) {
			FNode* node = AllocNode();
			new(&node->value_) ElementType(std::forward<ArgTypes>(Args)...);

			head_->next_.store(node, std::memory_order_release);
			head_ = node;
		}

		// returns empty TOptional if queue is empty 
		std::optional<ElementType> Dequeue() {
			FNode* local_tail = tail_.load(std::memory_order_relaxed);
			FNode* local_tail_next = local_tail->next_.load(std::memory_order_acquire);
			if (local_tail_next == nullptr) {
				return {};
			}

			ElementType* tail_next_value = reinterpret_cast<ElementType*>(&local_tail_next->value_);
			std::optional<ElementType> Value{ std::move(*tail_next_value) };
			DestructItem(tail_next_value);

			tail_.store(local_tail_next, std::memory_order_release);
			return Value;
		}

		bool Dequeue(ElementType& out_element) {
			std::optional<ElementType> local_element = Dequeue();
			if (local_element.has_value()) {
				out_element = local_element.GetValue();
				return true;
			}

			return false;
		}

		bool IsEmpty() const {
			FNode* local_tail = tail_.load(std::memory_order_relaxed);
			FNode* local_tail_next = local_tail->next_.load(std::memory_order_acquire);
			return local_tail_next == nullptr;
		}

		// as there can be only one consumer, a consumer can safely "peek" the tail of the queue.
		// returns a pointer to the tail if the queue is not empty, nullptr otherwise
		// there's no overload with TOptional as it doesn't support references
		ElementType* Peek() const {
			FNode* local_tail = tail_.load(std::memory_order_relaxed);
			FNode* local_tail_next = local_tail->next_.load(std::memory_order_acquire);

			if (local_tail_next == nullptr) {
				return nullptr;
			}

			return (ElementType*)&local_tail_next->value_;
		}

	private:
		struct TTypeCompatibleBytes {
			using ElementTypeAlias_NatVisHelper = ElementType;
			ElementType* GetTypedPtr() { return (ElementType*)this; }
			const ElementType* GetTypedPtr() const { return (const ElementType*)this; }

			alignas(ElementType) uint8_t Pad[sizeof(ElementType)];
		};

		struct FNode {
			std::atomic<FNode*> next_{ nullptr };
			TTypeCompatibleBytes value_;
		};

	private:
		void DestructItem(ElementType* Element) {
			if constexpr (!std::is_trivially_destructible_v<ElementType>) {
				// We need a typedef here because VC won't compile the destructor call below if ElementType itself has a member called ElementType
				typedef ElementType DestructItemsElementTypeTypedef;

				Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
			}
		}

		FNode* AllocNode() {
			// first tries to allocate node from internal node cache, 
			// if attempt fails, allocates node via ::operator new() 

			if (first_ != tail_copy_) {
				return AllocateFromCache();
			}

			tail_copy_ = tail_.load(std::memory_order_acquire);
			if (first_ != tail_copy_) {
				return AllocateFromCache();
			}

			return new FNode();
		}

		FNode* AllocateFromCache() {
			FNode* node = first_;
			first_ = first_->next_;
			node->next_.store(nullptr, std::memory_order_relaxed);
			return node;
		}

	private:
		std::atomic<FNode*> tail_; // tail of the queue 
		FNode* head_; // head of the queue
		FNode* first_; // last unused node (tail of node cache) 
		FNode* tail_copy_; // helper (points somewhere between first_ and tail_)
	};
}