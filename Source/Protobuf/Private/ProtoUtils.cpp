#include "stdafx.h"
#include "Protobuf/Public/ProtoUtils.h"
#include "Core/System/Tick.h"

namespace Protobuf {
	google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint) {
		return google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(timepoint.GetPosixTimeMicroSeconds());
	}

	google::protobuf::Timestamp ToTimestamp(const System::Tick& tick) {
		return google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(tick.GetEpocMicroseconds());
	}

	System::Time ToTime(const google::protobuf::Timestamp& timestamp) {
		return System::Time::FromPosixTimeMicroSeconds(google::protobuf::util::TimeUtil::TimestampToMicroseconds(timestamp));
	}

	System::Tick ToTick(const google::protobuf::Timestamp& timestamp) {
		return System::Tick::FromEpocMicroseconds(google::protobuf::util::TimeUtil::TimestampToMicroseconds(timestamp));
	}
} // namespace Protobuf