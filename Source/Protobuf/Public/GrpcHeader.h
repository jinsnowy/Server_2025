#pragma once

#include "Core/ThirdParty/WarningMacros.h"

PROTOBUF_IGNORE_WARNINGS_PUSH

#include <grpc/grpc.h>
#include <grpc++/grpc++.h>
#include <grpcpp/grpcpp.h>
#include <absl/strings/string_view.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

PROTOBUF_IGNORE_WARNINGS_POP
