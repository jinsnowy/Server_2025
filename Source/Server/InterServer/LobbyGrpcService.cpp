#include "stdafx.h"
#include "LobbyGrpcService.h"
#include "../Database/DB.h"
#include "../Model/Server.h"
#include "../Model/Account.h"
#include "../Model/Character.h"
#include "../Authenticator/Authenticator.h"

namespace Server {
	using System::Future;

	template<typename _Request, typename _Response>
	using GrpcHandler = System::Future<_Response> (*)(::grpc::ServerContext*, const _Request&);

	template<typename _Request, typename _Response>
	using AsyncServiceRequest = void (lobby_service::LobbyService::AsyncService::*)(
		grpc::ServerContext*, 
		_Request*,
		grpc::ServerAsyncResponseWriter<_Response>*,
		grpc::CompletionQueue*,
		grpc::ServerCompletionQueue*,
		void*);

	class AsyncCallData {
	public:
		AsyncCallData(System::Channel channel) : channel_(std::move(channel)) {}

		virtual void Proceed() = 0;
		virtual ~AsyncCallData() = default;

		System::Channel& GetChannel() {
			return channel_;
		}

	protected:
		System::Channel channel_;
	};

	enum AsyncCallStatus {
		CREATE,
		PROCESS,
		FINISH
	};

	template<typename _Request, typename _Response>
	class AsyncCallDataImpl : public AsyncCallData {
	public:
		static int call_count;

		AsyncCallDataImpl(lobby_service::LobbyService::AsyncService* service, 
			grpc::ServerCompletionQueue* cq,
			GrpcHandler<_Request, _Response> handler,
			AsyncServiceRequest<_Request, _Response> async_service_request,
			System::Channel channel)
			: 
			AsyncCallData(channel),
			service_(service),
			cq_(cq), 
			responder_(&ctx_), 
			status_(CREATE),
			handler_(handler) ,
			async_service_request_(async_service_request)
		{
		}

		void Proceed() override {
			if (status_ == CREATE) {
				status_ = PROCESS;
				(service_->*async_service_request_)(&ctx_, &request_, &responder_, cq_, cq_, this);
			}
			else if (status_ == PROCESS) {

				auto next_call = new AsyncCallDataImpl<_Request, _Response>(service_, cq_, handler_, async_service_request_, channel_); // Create a new instance for the next call
				next_call->Proceed(); // Start the next call

				System::Future<_Response> response = handler_(&ctx_, request_);
				response.Then([this](_Response response) {
					DEBUG_ASSERT(this->GetChannel().IsSynchronized());
					responder_.Finish(response, grpc::Status::OK, this);
					status_ = FINISH;
				}).Catch([this](const std::exception& error) {
					DEBUG_ASSERT(this->GetChannel().IsSynchronized());
					LOG_ERROR("Error in gRPC handler: {}", error.what());
					_Response response;
					responder_.Finish(response, grpc::Status(grpc::StatusCode(GRPC_STATUS_INTERNAL), error.what()), this);
					status_ = FINISH;
				});
			}
			else {
				delete this; // Cleanup
			}
		}

	private:
		lobby_service::LobbyService::AsyncService* service_;
		grpc::ServerCompletionQueue* cq_;
		grpc::ServerContext ctx_;
		_Request request_;
		grpc::ServerAsyncResponseWriter<_Response> responder_;
		AsyncCallStatus status_;
		GrpcHandler<_Request, _Response> handler_;
		AsyncServiceRequest<_Request, _Response> async_service_request_;
	};

	template<typename _Request, typename _Response>
	int AsyncCallDataImpl<_Request, _Response>::call_count = 0;

	class LobbyGrpcServiceCallback : public System::Singleton<LobbyGrpcServiceCallback> {
	public:
		template<typename _Request, typename _Response>
		void RegisterHandler(AsyncServiceRequest<_Request, _Response> async_service_requset, GrpcHandler<_Request, _Response> handler) {
			const AsyncCallDataFactory factory = [handler, async_service_requset](lobby_service::LobbyService::AsyncService* service, grpc::ServerCompletionQueue* cq) -> AsyncCallData* {
				return new AsyncCallDataImpl<_Request, _Response>(service, cq, handler, async_service_requset, System::Channel::RoundRobin());
			};

			async_call_data_factories_.push_back(factory);
		}

		void BeginAccept(lobby_service::LobbyService::AsyncService* service, grpc::ServerCompletionQueue* cq) {
			for(const auto& factory : async_call_data_factories_) {
				AsyncCallData* call_data = factory(service, cq);
				call_data->Proceed(); // Start the first call
			}
		}

	private:
		using AsyncCallDataFactory = std::function<AsyncCallData*(lobby_service::LobbyService::AsyncService*, grpc::ServerCompletionQueue*)>;
		std::vector<AsyncCallDataFactory> async_call_data_factories_;
	};

	void LobbyGrpcService::StartAsyncService() {
		System::Scheduler::CreateThread([]() {
			LobbyGrpcService service;
			grpc::ServerBuilder builder;
			builder.AddListeningPort(LobbyGrpcService::kListenAddress, grpc::InsecureServerCredentials());
			builder.RegisterService(&service);

			LOG_INFO("LobbyGrpcService started on : {}", LobbyGrpcService::kListenAddress);

			std::unique_ptr<grpc::ServerCompletionQueue> cq = builder.AddCompletionQueue();
			std::unique_ptr<grpc::Server> server = builder.BuildAndStart();

			RegisterHandlers(LobbyGrpcServiceCallback::GetInstance());
			LobbyGrpcServiceCallback::GetInstance().BeginAccept(&service, cq.get());

			void* tag; // Completion Queue에서 반환된 태그
			bool ok = true;
			while (true) {
				auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(1);
				grpc::CompletionQueue::NextStatus status = cq->AsyncNext(&tag, &ok, deadline); // Add timeout for graceful shutdown if needed
				if (status == grpc::CompletionQueue::SHUTDOWN) {
					LOG_INFO("Completion queue is shutting down.");
					break; // Exit the loop if the server is shutting down
				}

				if (status == grpc::CompletionQueue::TIMEOUT) {
					// Handle timeout if necessary, e.g., check for server shutdown signal
					continue;
				}

				if (!ok) {
					LOG_ERROR("Completion queue returned an error or shutdown.");
					continue; // Skip processing if the call was not successful
				}

				// 태그를 기반으로 요청 처리
				
				static_cast<AsyncCallData*>(tag)->GetChannel().Post([tag]() {
					static_cast<AsyncCallData*>(tag)->Proceed(); // Proceed with the call
				});
			}
		});
	}

	using namespace lobby_service;

	struct LobbyServiceCallbackImpl
	{
		static Future<RegisterServerReponse> RegisterServer(::grpc::ServerContext*, const RegisterServerRequest& req) {
			RegisterServerReponse res;

			auto agent = DB::GetLobbyDB().GetAgent();
			Model::Server server;
			server.server_address = req.server_address();
			server.server_type = static_cast<Model::ServerType>(req.server_type());
			server.created_at = System::Time::UtcNow();

			if (server.UpsertToDb(*agent) == false) {
				LOG_ERROR("Failed to upsert server: server_address: {}, server_type: {}", server.server_address, System::Enums::ToString(server.server_type));
				res.set_result(types::Result::kDatabaseError);
				return System::FromResult(res);
			}

			return System::FromResult(res);
		}

		static Future<PingResponse> Ping(::grpc::ServerContext*, const PingRequest&) {
			return System::FromResult(PingResponse{});
		}

		static Future<CharacterLoginResponse> CharacterLogin(::grpc::ServerContext*, const CharacterLoginRequest& req) {

			 Future<CharacterLoginResponse> future;

			 CharacterLoginResponse res;

			 const std::string access_token = req.access_token();

			 auto agent = DB::GetLobbyDB().GetAgent();
			 auto load_account_result = Model::Account::LoadByAccessToken(*agent, access_token);
			 if (load_account_result.has_value() == false) {
				 LOG_ERROR("Failed to load account by access token: {}", access_token);
				 res.set_result(types::Result::kNotFound);
				 future.SetResult(res);
				 return future;
			 }

			 const int64_t account_id = load_account_result->account_id();
			 const int64_t character_id = req.playing_character_id();
			 const int32_t server_id = req.playing_server_id();

			 bool is_exists = Model::Character::IsCharacterExists(*agent, account_id, server_id, character_id);
			 if (is_exists == true) {
				 LOG_ERROR("Character not found for account_id: {}, server_id: {}, character_id: {}", account_id, server_id, character_id);
				 res.set_result(types::Result::kNotFound);
				 future.SetResult(res);
				 return future;
			 }

			 Model::Account& account = load_account_result.value();
			 account.SetLoginToDb(*agent, System::Time::UtcNow());

			 res.set_account_id(account.account_id());
			 future.SetResult(res);

			 return future;
		}
	};



	void LobbyGrpcService::RegisterHandlers(LobbyGrpcServiceCallback& callback) {
		callback.RegisterHandler(&lobby_service::LobbyService::AsyncService::RequestRegisterServer, &LobbyServiceCallbackImpl::RegisterServer);
		callback.RegisterHandler(&lobby_service::LobbyService::AsyncService::RequestPing, &LobbyServiceCallbackImpl::Ping);
		callback.RegisterHandler(&lobby_service::LobbyService::AsyncService::RequestCharacterLogin, &LobbyServiceCallbackImpl::CharacterLogin);
	}

	

}

