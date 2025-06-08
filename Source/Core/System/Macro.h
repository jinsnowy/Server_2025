#pragma once

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
		else {\
			std::abort();\
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

#define FORMAT(...) std::format(__VA_ARGS__)

#define NO_COPY_AND_ASSIGN(Class)\
Class(const Class&) = delete;\
Class& operator=(const Class&) = delete;
