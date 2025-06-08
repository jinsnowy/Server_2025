#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cassert>
#include <csignal>

#include <iostream>
#include <iomanip>
#include <sstream>
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

#ifdef _WIN32
#undef CreateDirectory
#undef SendMessage
#undef PostMessage
#endif 

#include <boost/asio.hpp>
#include "Core/Logging/Logger.h"
#include "Core/System/Macro.h"
#include "Core/System/FuncTraits.h"
#include "Core/System/Callable.h"
#include "Core/System/DateTime.h"
#include "Core/System/TimePoint.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/System/Actor.h"
#include "Core/System/Actor.hpp"