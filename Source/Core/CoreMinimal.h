#pragma once

#include "Core/System/Singleton.h"
#include "Core/Logging/Logger.h"

#include "Core/System/Macro.h"
#include "Core/System/FuncTraits.h"
#include "Core/System/Callable.h"
#include "Core/System/Actor.h"
#include "Core/System/Channel.h"
#include "Core/System/Context.h"
#include "Core/System/Singleton.h"
#include "Core/System/Scheduler.h"
#include "Core/System/String.h"
#include "Core/System/DateTime.h"

#include "Core/Network/SessionFactory.h"
#include "Core/Network/Connection.h"
#include "Core/Network/Session.h"
#include "Core/Network/Listener.h"
#include "Core/Network/Socket.h"

#include "Core/ThirdParty/BoostAsio.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <sdkddkver.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#undef CreateDirectory
#undef SendMessage
#undef PostMessage

#endif 