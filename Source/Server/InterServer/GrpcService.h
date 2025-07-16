#pragma once

#include "Protobuf/Public/GrpcHeader.h"
#include "Core/System/Scheduler.h"

namespace Server 
{
    static std::vector<std::shared_ptr<grpc::Server>> sevices;

}