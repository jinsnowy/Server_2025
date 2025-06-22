#include "stdafx.h"
#include "Core/Container/SList.h"

namespace System {
	namespace Detail {

#ifdef _WIN32
		using SListHeader = SLIST_HEADER;
		using SListHeaderPointer = PSLIST_HEADER;
		using SListEntryPointer = PSLIST_ENTRY;

		using ElementDestructor = void(*)(void*);
		class SListImpl : public ISList {
		public:
			SListImpl(size_t allocation_size, ElementDestructor destructor);
			~SListImpl();

			void* Request() override;
			void Enqueue(void* data) override;
			void* Alloc() override;

		private:
			size_t allocation_size_;
			size_t node_size_;
			ElementDestructor destructor_;
			SListHeader header_;

			SListEntryPointer AsNode(void* data);
			void* AsData(SListEntryPointer node);
			void Destroy(SListEntryPointer node);
		};

		SListImpl::SListImpl(size_t allocation_size, ElementDestructor destructor)
			:
			allocation_size_(allocation_size),
			node_size_(allocation_size + sizeof(SLIST_ENTRY)),
			destructor_(destructor) {
			::InitializeSListHead(reinterpret_cast<SListHeaderPointer>(&header_));
		}

		SListImpl::~SListImpl() {
			for (;;) {
				SListEntryPointer node = ::InterlockedPopEntrySList(reinterpret_cast<SListHeaderPointer>(&header_));
				if (node == nullptr) {
					break;
				}
				Destroy(node);
			}
			::InterlockedFlushSList(reinterpret_cast<SListHeaderPointer>(&header_));
		}

		void SListImpl::Destroy(SListEntryPointer node) {
			destructor_(AsData(node));
			free(node);
		}

		SListEntryPointer SListImpl::AsNode(void* data) {
			return reinterpret_cast<SListEntryPointer>(data) - 1;
		}

		void* SListImpl::AsData(SListEntryPointer node) {
			return node + 1;
		}

		void SListImpl::Enqueue(void* node) {
			::InterlockedPushEntrySList(reinterpret_cast<SListHeaderPointer>(&header_), reinterpret_cast<SListEntryPointer>(node));
		}

		void* SListImpl::Request() {
			SListEntryPointer node = ::InterlockedPopEntrySList(reinterpret_cast<SListHeaderPointer>(&header_));
			if (node == nullptr) {
				return nullptr;
			}
			return AsData(node);
		}

		void* SListImpl::Alloc() {
			return _aligned_malloc(node_size_, MEMORY_ALLOCATION_ALIGNMENT);
		}

		std::unique_ptr<ISList> CreateSListImpl(size_t allocation_size, void(*destructor)(void*)) {
			return std::make_unique<SListImpl>(allocation_size, destructor);
		}
#else
		std::unique_ptr<ISList> CreateSListImpl(size_t allocation_size, std::function<void(void*)> deleter) {
			return nullptr;
		}
#endif
	}
}
