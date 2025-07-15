#include "stdafx.h"
#include "Protobuf/Public/ProtoUtils.h"

namespace Protobuf {
	google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint) {
		return google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(timepoint.GetPosixTimeMicroSeconds());
	}

	System::Time FromTimestamp(const google::protobuf::Timestamp& timestamp) {
		return System::Time::FromPosixTimeMicroSeconds(google::protobuf::util::TimeUtil::TimestampToMicroseconds(timestamp));
	}
} // namespace Protobuf