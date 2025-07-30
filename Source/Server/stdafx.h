#pragma once

#include <cstring>
#include <climits>
#include <cassert>

#include <iostream>
#include <string>
#include <fstream>

#include <algorithm>
#include <chrono>
#include <numeric>
#include <memory>
#include <limits>
#include <random>
#include <functional>
#include <utility>
#include <xutility>

#include <array>
#include <bitset>
#include <vector>
#include <queue>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <future>

// C++17/C++2x
#include <any>
#include <variant>
#include <optional>
#include <filesystem>
#include <concepts>
#include <coroutine>
#include <ranges>
#include <format>
#include <source_location>

#ifdef _MSC_VER
#include "Core/Platform/Windows.h"
#endif 

#include "Core/CoreMinimal.h"
#include "Core/ThirdParty/Protobuf.h"
#include "Core/ThirdParty/Sql.h"
#include "Core/Sql/Sql.h"
#include "Protobuf/Public/ProtoUtils.h"
#include "Protobuf/Public/User.h"
#include "Protobuf/Public/World.h"
#include "Protobuf/Public/Types.h"
#include "Core/Math/LinAlgebra.h"

PROTOBUF_IGNORE_WARNINGS_PUSH

#include <google/protobuf/message.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/util/time_util.h>

PROTOBUF_IGNORE_WARNINGS_POP

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

