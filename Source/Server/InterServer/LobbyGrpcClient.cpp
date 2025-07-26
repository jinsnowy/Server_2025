#include "stdafx.h"
#include "LobbyGrpcClient.h"
#include "GrpcService.h"

namespace Server {
    template<typename _Request, typename _Response>
    using GrpcCallImpl = std::unique_ptr<grpc::ClientAsyncResponseReader<_Response>>(lobby_service::LobbyService::Stub::*)(grpc::ClientContext*, const _Request&, grpc::CompletionQueue*);

    struct AsyncCallData {
        virtual void HandleCompletion(bool ok) = 0;
        virtual ~AsyncCallData() = default;
    };

    template<typename _Request, typename _Response>
    struct CallData : AsyncCallData {
        // The producer-consumer queue we use to communicate asynchronously with the gRPC runtime.
        grpc::CompletionQueue* cq_;
        lobby_service::LobbyService::Stub* stub_;
        grpc::ClientContext context;

        std::shared_ptr<_Request> request;
        _Response response;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<_Response>> response_reader;
        LobbyGrpcClient* client_instance;
        GrpcCallImpl<_Request, _Response> grpc_call_impl;
        System::Future<GrpcCallResult<_Response>> response_future;

        // Constructor to set up the RPC
        CallData(LobbyGrpcClient* client,
            grpc::CompletionQueue* cq,
            lobby_service::LobbyService::Stub* stub,
            const std::shared_ptr<_Request>& request_in,
            GrpcCallImpl<_Request, _Response> grpc_call_impl_in)
            :
            cq_(cq),
            stub_(stub),
            request(std::move(request_in)),
            client_instance(client),
            grpc_call_impl(grpc_call_impl_in)
        {

            // You can set a deadline for the RPC here if needed
            context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5)); // 5-second deadline

            // This is where you initiate the asynchronous RPC.
            // stub_->PrepareAsyncPing() creates the RPC object but doesn't start it yet.
            response_reader = (stub_->*grpc_call_impl)(&context, *request, cq_); // Empty request for Ping

            // Start the RPC call.
            response_reader->StartCall();

            // Request that, upon completion of the RPC, 'response' be updated with the server's response;
            // 'status' with the indication of whether the operation was successful.
            // We use 'this' (the address of this CallData object) as the unique tag.
            response_reader->Finish(&response, &status, this);
        }

        // This method is called when the RPC operation completes and its tag is returned from the CQ.
        void HandleCompletion(bool ok) override {
            if (!ok) {
                LOG_ERROR("Ping RPC completion queue event 'ok' is false.");
            }

			GrpcCallResult<_Response> grpc_result;
			grpc_result.response = std::move(response);
			grpc_result.status = status;

            response_future.SetResult(std::move(grpc_result));

            // Clean up this CallData object after processing
            delete this;
        }
    };

 
    class GrpcCallCallback : public System::Singleton<GrpcCallCallback> {
    public:
        template<typename _Request, typename _Response>
        void RegisterCallback(GrpcCallImpl<_Request, _Response> grpc_call_impl) {
            size_t key = System::hashcode<std::decay_t<_Request>>();
            async_callbacks[key] = [grpc_call_impl](lobby_service::LobbyService::Stub* stub, grpc::CompletionQueue* cq, const std::shared_ptr<google::protobuf::Message>& request) {
                return new CallData<_Request, _Response>(nullptr, cq, stub, std::static_pointer_cast<_Request>(request), grpc_call_impl);
            };
        }

        template<typename _Request, typename _Response>
        System::Future<GrpcCallResult<_Response>> GrpcCall(const std::shared_ptr<_Request>& request, grpc::CompletionQueue* cq, lobby_service::LobbyService::Stub* stub) {
            auto iter = async_callbacks.find(System::hashcode<std::decay_t<_Request>>());
            if (iter == async_callbacks.end()) {
                throw std::runtime_error("No callback registered for this request type.");
            }

            auto call_data = iter->second(stub, cq, request);
            return static_cast<CallData<_Request, _Response>*>(call_data)->response_future;
        }

    private:
        using AsyncCallDataFactory = std::function<AsyncCallData* (lobby_service::LobbyService::Stub*, grpc::CompletionQueue*, const std::shared_ptr<google::protobuf::Message>&)>;
        std::unordered_map<size_t, AsyncCallDataFactory> async_callbacks;
    };
  
	LobbyGrpcClient::LobbyGrpcClient(std::string lobby_server_address)
		:
		channel_(grpc::CreateChannel(lobby_server_address, grpc::InsecureChannelCredentials())),
		stub_(lobby_service::LobbyService::NewStub(channel_)),
		cq_(std::make_shared<grpc::CompletionQueue>()) {
		RegisterAsyncHandlers(GrpcCallCallback::GetInstance());
        System::Scheduler::CreateThread([cq = cq_]() {
            AsyncCompletionRoutine(cq);
		});
	}

    System::Future<GrpcCallResult<PingResponse>> LobbyGrpcClient::Ping(const std::shared_ptr<lobby_service::PingRequest>& request) {
		return GrpcCallCallback::GetInstance().GrpcCall<lobby_service::PingRequest, lobby_service::PingResponse>(request, cq_.get(), stub_.get());
    }

    System::Future<GrpcCallResult<CharacterLoginResponse>> LobbyGrpcClient::CharacterLogin(const std::shared_ptr<lobby_service::CharacterLoginRequest>& request) {
        return GrpcCallCallback::GetInstance().GrpcCall<lobby_service::CharacterLoginRequest, lobby_service::CharacterLoginResponse>(request, cq_.get(), stub_.get());
    }

    System::Future<GrpcCallResult<RegisterServerReponse>> LobbyGrpcClient::RegisterServer(const std::shared_ptr<lobby_service::RegisterServerRequest>& request) {
        return GrpcCallCallback::GetInstance().GrpcCall<lobby_service::RegisterServerRequest, lobby_service::RegisterServerReponse>(request, cq_.get(), stub_.get());
    }

    void LobbyGrpcClient::AsyncCompletionRoutine(std::shared_ptr<grpc::CompletionQueue> cq) {
		void* tag;
		bool ok;

        while (true) {
            // Block until the next result is available in the completion queue "cq_".
            // Use a timeout for graceful shutdown (e.g., if the client is being destroyed)
            auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(1);
            grpc::CompletionQueue::NextStatus status_cq = cq->AsyncNext(&tag, &ok, deadline);

            if (status_cq == grpc::CompletionQueue::SHUTDOWN) {
                LOG_INFO("Client Completion Queue shutting down. Exiting thread.");
                break;
            }
            if (status_cq == grpc::CompletionQueue::TIMEOUT) {
                // Periodically check for shutdown or perform other tasks
                continue;
            }

            // If we got an event (status_cq == GRPC_CQ_OK)
            // Retrieve the CallData object from the tag and call its completion handler.
            System::Scheduler::Any([tag, ok](auto&) {
                AsyncCallData* call_data = static_cast<AsyncCallData*>(tag);
                call_data->HandleCompletion(ok);
            });
        }
    }

    void LobbyGrpcClient::RegisterAsyncHandlers(GrpcCallCallback& callback)
    {
        callback.RegisterCallback(&lobby_service::LobbyService::Stub::PrepareAsyncPing);
        callback.RegisterCallback(&lobby_service::LobbyService::Stub::PrepareAsyncCharacterLogin);
        callback.RegisterCallback(&lobby_service::LobbyService::Stub::PrepareAsyncRegisterServer);
    }



}
