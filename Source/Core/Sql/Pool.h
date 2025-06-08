#pragma once

#include "Core/Container/SList.h"

namespace Sql  {
	class Agent;
	class Conn;
	class Pool final {
	public:
		static constexpr int32_t kMaxTryCount = 10;
		static constexpr int32_t kWaitMilliseconds = 100;

		Pool();
		~Pool();

		bool Connect(const wchar_t* connectionString, uint32_t connectionCount);
		uint32_t GetConnectionCount() const { return static_cast<uint32_t>(connections_.Count());  }
		void Clear();
		std::shared_ptr<Conn> Dequeue();
		void Enqueue(std::shared_ptr<Conn>&& conn);

	private:
		struct Internal;
		std::unique_ptr<Internal> _internal;
		std::wstring _connStr;
		System::SList<std::shared_ptr<Conn>> connections_;
	};
}