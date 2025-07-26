#pragma once

#include "Protobuf/Public/LobbyService.h"

using namespace lobby_service;

namespace Server {
	class GrpcCallCallback;

	template<typename Response>
	struct GrpcCallResult {
		Response response;
		grpc::Status status;

		bool ok() const {
			return status.ok();
		}
	};

	class LobbyGrpcClient {
	public:
		LobbyGrpcClient(std::string lobby_server_address);

		System::Future<GrpcCallResult<PingResponse>> Ping(const std::shared_ptr<lobby_service::PingRequest>& request);
		System::Future<GrpcCallResult<CharacterLoginResponse>> CharacterLogin(const std::shared_ptr<lobby_service::CharacterLoginRequest>& request);
		System::Future<GrpcCallResult<RegisterServerReponse>> RegisterServer(const std::shared_ptr<lobby_service::RegisterServerRequest>& request);

		lobby_service::LobbyService::Stub& GetStub() {
			return *stub_;
		}
	
	private:
		std::shared_ptr<grpc::CompletionQueue> cq_;
		std::shared_ptr<grpc::Channel> channel_;
		std::unique_ptr<lobby_service::LobbyService::Stub> stub_;

		static void AsyncCompletionRoutine(std::shared_ptr<grpc::CompletionQueue> cq);
		static void RegisterAsyncHandlers(GrpcCallCallback& callback);
	};
}
