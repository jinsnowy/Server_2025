#pragma once

#include "Core/ThirdParty/BoostAsio.h"
#include "Core/System/Actor.h"

namespace System{
	class Context;
} // namespace System

namespace Network {
	class Session;
	class SessionFactory;
	class Socket;
	class Resolver;
	class Session;
	class Buffer;
	class Protocol;
	class BufferView;
	struct RecvNetworkStream;
	struct SendNetworkStream;
	class Connection final : public System::Actor {
	public:
		Connection(std::unique_ptr<Socket> socket);
		Connection(std::shared_ptr<System::Context> context);
		~Connection();

		void Connect(const std::string& ip, const uint16_t& port, std::shared_ptr<Session> session);
		void Disconnect();
	
		bool IsConnected() const;
		void Send(const BufferView& buffer);
		bool IsSendInProgress() const;

		std::string ToString() const;
		const std::unique_ptr<Socket>& socket() const { return socket_; }
		void set_session(std::shared_ptr<Session> session) { session_ = session; }

	private:
		friend class SessionFactory;

		std::unique_ptr<Socket> socket_;
		std::unique_ptr<Resolver> resolver_;
		std::weak_ptr<Session> session_;
		std::shared_ptr<Protocol> protocol_;

		std::unique_ptr<SendNetworkStream> send_stream_;
		std::unique_ptr<RecvNetworkStream> recv_stream_;

		std::string ip_;
		uint16_t port_ = 0;

		void BeginReceive();

		void OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results);
		void OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint);
		void OnReceived(const boost::system::error_code& error, std::size_t bytes_transferred);
		void OnSendCompleted(const boost::system::error_code& error, std::size_t bytes_transferred);
		void FlushSend(bool continueOnWriter = false);

		bool ReceiveImpl(const size_t length);
		void SendImpl(const BufferView& buffer);
	
	};
}
