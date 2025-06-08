#pragma once

#ifdef _WIN32

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define WIN

#include <windows.h>

#undef CreateDirectory
#undef SendMessage
#undef PostMessage

#endif