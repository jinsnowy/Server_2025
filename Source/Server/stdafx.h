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

#include "Core/Platform/Windows.h"
#include "Core/CoreMinimal.h"
#include "Core/ThirdParty/Protobuf.h"

PROTOBUF_IGNORE_WARNINGS_PUSH

#include <google/protobuf/message.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/util/time_util.h>

PROTOBUF_IGNORE_WARNINGS_POP

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

#ifdef _WIN32

#pragma comment(lib, "boost_system-vc143-mt-gd-x64-1_88.lib")
#pragma comment(lib, "boost_filesystem-vc143-mt-gd-x64-1_88.lib")
#pragma comment(lib, "boost_locale-vc143-mt-gd-x64-1_88.lib")
#pragma comment(lib, "abseil_dll.lib")
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "libcurl-d.lib")
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "gpr.lib")
#pragma comment(lib, "grpc.lib")
#pragma comment(lib, "grpc++.lib")
#pragma comment(lib, "grpc++_unsecure.lib")

#endif