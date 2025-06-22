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
#include <xutility> // 매우 중요

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
#include "Core/ThirdParty/Sql.h"
#include "Core/Sql/Sql.h"

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

#ifndef NDEBUG
#pragma comment(lib, "boost_system-vc143-mt-gd-x64-1_88.lib")
#pragma comment(lib, "boost_filesystem-vc143-mt-gd-x64-1_88.lib")
#pragma comment(lib, "boost_locale-vc143-mt-gd-x64-1_88.lib")

#pragma comment(lib, "abseil_dll.lib")
#pragma comment(lib, "absl_decode_rust_punycode.lib")
#pragma comment(lib, "absl_demangle_rust.lib")
#pragma comment(lib, "absl_flags_commandlineflag.lib")
#pragma comment(lib, "absl_flags_commandlineflag_internal.lib")
#pragma comment(lib, "absl_flags_config.lib")
#pragma comment(lib, "absl_flags_internal.lib")
#pragma comment(lib, "absl_flags_marshalling.lib")
#pragma comment(lib, "absl_flags_parse.lib")
#pragma comment(lib, "absl_flags_private_handle_accessor.lib")
#pragma comment(lib, "absl_flags_program_name.lib")
#pragma comment(lib, "absl_flags_reflection.lib")
#pragma comment(lib, "absl_flags_usage.lib")
#pragma comment(lib, "absl_flags_usage_internal.lib")
#pragma comment(lib, "absl_log_flags.lib")
#pragma comment(lib, "absl_log_internal_structured_proto.lib")
#pragma comment(lib, "absl_poison.lib")
#pragma comment(lib, "absl_tracing_internal.lib")
#pragma comment(lib, "absl_utf8_for_code_point.lib")
#pragma comment(lib, "address_sorting.lib")

#pragma comment(lib, "bz2d.lib")
#pragma comment(lib, "cares.lib")
#pragma comment(lib, "ffi.lib")
#pragma comment(lib, "gpr.lib")
#pragma comment(lib, "grpc.lib")
#pragma comment(lib, "grpc_authorization_provider.lib")
#pragma comment(lib, "grpc_plugin_support.lib")
#pragma comment(lib, "grpc_unsecure.lib")
#pragma comment(lib, "grpc++.lib")
#pragma comment(lib, "grpc++_alts.lib")
#pragma comment(lib, "grpc++_error_details.lib")
#pragma comment(lib, "grpc++_reflection.lib")
#pragma comment(lib, "grpc++_unsecure.lib")
#pragma comment(lib, "grpcpp_channelz.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libexpatd.lib")
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "libprotobuf-lited.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "lzma.lib")
#pragma comment(lib, "upb_base_lib.lib")
#pragma comment(lib, "upb_json_lib.lib")
#pragma comment(lib, "upb_mem_lib.lib")
#pragma comment(lib, "upb_message_lib.lib")
#pragma comment(lib, "upb_mini_descriptor_lib.lib")
#pragma comment(lib, "upb_textformat_lib.lib")
#pragma comment(lib, "upb_wire_lib.lib")
#pragma comment(lib, "utf8_range.lib")
#pragma comment(lib, "utf8_validity.lib")
#pragma comment(lib, "zstd.lib")
#pragma comment(lib, "re2.lib")

// CURL
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libcurl-d.lib")

// ODBC
#pragma comment(lib, "odbc32.lib")

// GameServer Dependent Project
#pragma comment(lib, "Core.lib")
#pragma comment(lib, "Protobuf.lib")

#endif

#endif
