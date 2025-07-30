#pragma once

#ifdef _MSC_VER

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define WIN

#include <windows.h>
#include <winsdkver.h>

#undef CreateDirectory
#undef SendMessage
#undef PostMessage

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_MAXVER // Target Windows 10
#endif

#endif