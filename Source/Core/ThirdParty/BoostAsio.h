#pragma once

#include "Core/ThirdParty/WarningMacros.h"

BOOST_IGNORE_WARNINGS_PUSH

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/tcp.hpp>

BOOST_IGNORE_WARNINGS_POP

namespace boost::asio {
	class io_context;
}  // namespace boost::asio
