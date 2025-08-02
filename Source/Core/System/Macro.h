#pragma once

#ifdef __has_attribute
#define HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAVE_ATTRIBUTE(x) 0
#endif

// ABSL_HAVE_CPP_ATTRIBUTE
//
// A function-like feature checking macro that accepts C++11 style attributes.
// It's a wrapper around `__has_cpp_attribute`, defined by ISO C++ SD-6
// (https://en.cppreference.com/w/cpp/experimental/feature_test). If we don't
// find `__has_cpp_attribute`, will evaluate to 0.
#if defined(__cplusplus) && defined(__has_cpp_attribute)
// NOTE: requiring __cplusplus above should not be necessary, but
// works around https://bugs.llvm.org/show_bug.cgi?id=23435.
#define HAVE_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define HAVE_CPP_ATTRIBUTE(x) 0
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#define ASSERT(expr) _ASSERT(expr)
#else
#define ASSERT(expr) do{ (expr); } while(false)
#endif

#define DEBUG_ASSERT(expr)\
do{\
	if (!(expr)) {\
		if (IsDebuggerPresent()) {\
			__debugbreak();\
		}\
	}\
	__assume(expr);\
} while (false)

#define RELEASE_ASSERT(expr)\
do {\
	if (!(expr)) {\
		std::abort();\
	}\
	__assume(expr);\
} while (false)

#define DEBUG_BREAK \
if (IsDebuggerPresent()) {\
			__debugbreak();\
}

#define UNREACHABLE()\
do {\
	assert(false && "Unreachable code reached!");\
	__assume(false);\
} while (false)


#ifdef FALLTHROUGH_INTENDED
#error "FALLTHROUGH_INTENDED should not be defined."
#elif HAVE_CPP_ATTRIBUTE(fallthrough)
#define FALLTHROUGH_INTENDED [[fallthrough]]
#elif HAVE_CPP_ATTRIBUTE(clang::fallthrough)
#define FALLTHROUGH_INTENDED [[clang::fallthrough]]
#elif HAVE_CPP_ATTRIBUTE(gnu::fallthrough)
#define FALLTHROUGH_INTENDED [[gnu::fallthrough]]
#else
#define FALLTHROUGH_INTENDED \
  do {                            \
  } while (0)
#endif

#define FORMAT(...) std::format(__VA_ARGS__)

#define NO_COPY_AND_ASSIGN(Class)\
Class(const Class&) = delete;\
Class& operator=(const Class&) = delete;

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define __FILELINE__ __FILE__ "(" STRINGIFY(__LINE__) ")"
