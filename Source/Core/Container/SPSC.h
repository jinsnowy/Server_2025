#pragma once
/**
 * Fast single-producer/single-consumer unbounded concurrent queue. Doesn't free memory until destruction but recycles consumed items.
 * Based on http://www.1024cores.net/home/lock-free-algorithms/queues/unbounded-spsc-queue
 */

namespace Container {

	template<typename T>
	class SPSCQueue final
	{
	public:
		using ElementType = T;

		SPSCQueue(const SPSCQueue&) = delete;
		SPSCQueue& operator=(const SPSCQueue&) = delete;

		SPSCQueue()
		{
			FNode* Node = new FNode();
			Tail.store(Node, std::memory_order_relaxed);
			Head = First = TailCopy = Node;
		}

		~SPSCQueue()
		{
			FNode* Node = First;
			FNode* LocalTail = Tail.load(std::memory_order_relaxed);

			// Delete all nodes which are the sentinel or unoccupied
			bool bContinue = false;
			do
			{
				FNode* Next = Node->Next.load(std::memory_order_relaxed);
				bContinue = Node != LocalTail;
				delete Node;
				Node = Next;
			} while (bContinue);

			// Delete all nodes which are occupied, destroying the element first
			while (Node != nullptr)
			{
				FNode* Next = Node->Next.load(std::memory_order_relaxed);
				DestructItem((ElementType*)&Node->Value);
				delete Node;
				Node = Next;
			}
		}

		template <typename... ArgTypes>
		void Enqueue(ArgTypes&&... Args)
		{
			FNode* Node = AllocNode();
			new(&Node->Value) ElementType(std::forward<ArgTypes>(Args)...);

			Head->Next.store(Node, std::memory_order_release);
			Head = Node;
		}

		// returns empty TOptional if queue is empty 
		std::optional<ElementType> Dequeue()
		{
			FNode* LocalTail = Tail.load(std::memory_order_relaxed);
			FNode* LocalTailNext = LocalTail->Next.load(std::memory_order_acquire);
			if (LocalTailNext == nullptr)
			{
				return {};
			}

			ElementType* TailNextValue = (ElementType*)&LocalTailNext->Value;
			std::optional<ElementType> Value{ std::move(*TailNextValue) };
			DestructItem(TailNextValue);

			Tail.store(LocalTailNext, std::memory_order_release);
			return Value;
		}

		bool Dequeue(ElementType& OutElem)
		{
			std::optional<ElementType> LocalElement = Dequeue();
			if (LocalElement.IsSet())
			{
				OutElem = LocalElement.GetValue();
				return true;
			}

			return false;
		}

		bool IsEmpty() const
		{
			FNode* LocalTail = Tail.load(std::memory_order_relaxed);
			FNode* LocalTailNext = LocalTail->Next.load(std::memory_order_acquire);
			return LocalTailNext == nullptr;
		}

		// as there can be only one consumer, a consumer can safely "peek" the tail of the queue.
		// returns a pointer to the tail if the queue is not empty, nullptr otherwise
		// there's no overload with TOptional as it doesn't support references
		ElementType* Peek() const
		{
			FNode* LocalTail = Tail.load(std::memory_order_relaxed);
			FNode* LocalTailNext = LocalTail->Next.load(std::memory_order_acquire);

			if (LocalTailNext == nullptr)
			{
				return nullptr;
			}

			return (ElementType*)&LocalTailNext->Value;
		}

	private:
		struct TTypeCompatibleBytes
		{
			using ElementTypeAlias_NatVisHelper = ElementType;
			ElementType* GetTypedPtr() { return (ElementType*)this; }
			const ElementType* GetTypedPtr() const { return (const ElementType*)this; }

			alignas(ElementType) uint8_t Pad[sizeof(ElementType)];
		};

		struct FNode
		{
			std::atomic<FNode*> Next{ nullptr };
			TTypeCompatibleBytes Value;
		};

	private:
		void DestructItem(ElementType* Element)
		{
			if constexpr (!std::is_trivially_destructible_v<ElementType>)
			{
				// We need a typedef here because VC won't compile the destructor call below if ElementType itself has a member called ElementType
				typedef ElementType DestructItemsElementTypeTypedef;

				Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
			}
		}

		FNode* AllocNode()
		{
			// first tries to allocate node from internal node cache, 
			// if attempt fails, allocates node via ::operator new() 

			auto AllocFromCache = [this]()
				{
					FNode* Node = First;
					First = First->Next;
					Node->Next.store(nullptr, std::memory_order_relaxed);
					return Node;
				};

			if (First != TailCopy)
			{
				return AllocFromCache();
			}

			TailCopy = Tail.load(std::memory_order_acquire);
			if (First != TailCopy)
			{
				return AllocFromCache();
			}

			return new FNode();
		}

	private:
		std::atomic<FNode*> Tail; // tail of the queue 
		FNode* Head; // head of the queue
		FNode* First; // last unused node (tail of node cache) 
		FNode* TailCopy; // helper (points somewhere between First and Tail)
	};
}