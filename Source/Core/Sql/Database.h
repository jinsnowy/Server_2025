#pragma once

#include "Core/Sql/Agent.h"

namespace Sql  {
	class Conn;
	class Pool;

	void Initialize(const std::wstring& connectionString, uint32_t connectionCount);

	void Destroy();

	Pool& GetPool();

	Agent GetAgent();
}
