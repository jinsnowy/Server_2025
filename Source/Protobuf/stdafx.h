#pragma once

#include <cstring>
#include <climits>
#include <cassert>

#include <iostream>
#include <string>
#include <fstream>

#include <algorithm>
#include <vector>
#include <queue>
#include <deque>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <variant>
#include <exception>

// C++17/C++2x
#include <any>
#include <variant>
#include <optional>
#include <filesystem>
#include <concepts>
#include <coroutine>
#include <ranges>
#include <format>

#include "Core/ThirdParty/WarningMacros.h"
#include "Core/Platform/Windows.h"

PROTOBUF_IGNORE_WARNINGS_PUSH

#include "google/protobuf/message.h"
#include "google/protobuf/timestamp.pb.h"
#include "google/protobuf/util/time_util.h"

PROTOBUF_IGNORE_WARNINGS_POP

#include "Core/CoreMinimal.h"