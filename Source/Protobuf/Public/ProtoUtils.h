#pragma once

#include "Core/System/Time.h"
#include "Core/ThirdParty/WarningMacros.h"

PROTOBUF_IGNORE_WARNINGS_PUSH

#include <google/protobuf/timestamp.pb.h>

PROTOBUF_IGNORE_WARNINGS_POP

namespace Protobuf {
	google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint);
	System::Time FromTimestamp(const google::protobuf::Timestamp& timestamp);
}

