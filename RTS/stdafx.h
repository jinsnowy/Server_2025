#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cassert>

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

#include <type_traits>
#include <typeinfo>
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
#include <shared_mutex>
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <sdkddkver.h>
#endif 

#include "Core/CoreMinimal.h"

#pragma comment(lib, "boost_system-vc143-mt-x64-1_88.lib")
#pragma comment(lib, "boost_filesystem-vc143-mt-x64-1_88.lib")
#pragma comment(lib, "boost_locale-vc143-mt-x64-1_88.lib")
#pragma comment(lib, "fmt.lib")