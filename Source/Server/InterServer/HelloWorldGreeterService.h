#pragma once

#include "Protobuf/Public/GrpcHeader.h"
#include "Protobuf/Public/HelloWorld.h"

namespace Server {
    class HelloWorldGreeterService final : public helloworld::Greeter::Service {
    public:
        grpc::Status SayHello(grpc::ServerContext*, const helloworld::HelloRequest* request, helloworld::HelloReply* reply) override {
            std::string prefix = "Hello, ";
            reply->set_message(prefix + request->name());
            return grpc::Status::OK;
        }
    };
}