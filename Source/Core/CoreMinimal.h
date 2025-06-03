#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX
#include <sdkddkver.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#undef CreateDirectory
#undef SendMessage
#undef PostMessage

#endif 

#include "Core/Logging/Logger.h"
#include "Core/System/Interface.h"
#include "Core/Network/Interface.h"

#include "Core/ThirdParty/BoostAsio.h"