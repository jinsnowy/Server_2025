#pragma once

#include "Core/ThirdParty/BoostAsio.h"
#include "Core/System/Actor.h"
#include "Core/Network/IPAddress.h"

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
	struct PacketSegment;
	class IPAddress;
	class SendNode;
	class OutputStream;
	class Connection final : public System::Actor {
	public:
		Connection(std::unique_ptr<Socket> socket);
		Connection(std::shared_ptr<System::Context> context);
		~Connection();

		void Connect(const std::string& ip, const uint16_t& port);
		void Disconnect();
	
		bool IsConnected() const;
		void Send(std::unique_ptr<SendNode> node);

		std::string ToString() const;
		const std::unique_ptr<Socket>& socket() const { return socket_; }

		const std::unique_ptr<Protocol>& protocol() const { return protocol_; }

		const IPAddress& connected_address() const {
			return connceted_address_;
		}

		System::Delegate<void(std::shared_ptr<Connection>, const IPAddress&)>& on_connected_delegate() {
			return on_connected_delegate_;
		}

		System::Delegate<void(const IPAddress&, const std::string&)>& on_connect_failed_delegate() {
			return on_connect_failed_delegate_;
		}

		System::Delegate<void()>& on_disconnected_delegate() {
			return on_disconnected_delegate_;
		}

		System::Delegate<void()>& on_data_received_delegate() {
			return on_data_received_delegate_;
		}

		std::unique_ptr<OutputStream>& output_stream() {
			return output_stream_;
		}

		const std::unique_ptr<OutputStream>& output_stream() const {
			return output_stream_;
		}

		System::Function<std::unique_ptr<Protocol>()>& protocol_factory() {
			return protocol_factory_;
		}

		void BeginConnection();

	private:
		std::unique_ptr<Socket> socket_;
		std::unique_ptr<Resolver> resolver_;

		System::Function<std::unique_ptr<Protocol>()> protocol_factory_;
		System::Delegate<void(std::shared_ptr<Connection>, const IPAddress&)> on_connected_delegate_;
		System::Delegate<void(const IPAddress&, const std::string&)> on_connect_failed_delegate_;
		System::Delegate<void()> on_disconnected_delegate_;
		System::Delegate<void()> on_data_received_delegate_;

		std::unique_ptr<SendNetworkStream> send_stream_;
		std::unique_ptr<RecvNetworkStream> recv_stream_;
		std::unique_ptr<Protocol> protocol_;
		std::unique_ptr<OutputStream> output_stream_;

		IPAddress target_address_;
		IPAddress connceted_address_;
		bool is_sending_ = false;

		void BeginReceive();

		void OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results);
		void OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint);
		void OnDisconnected();
		void OnReceived(const boost::system::error_code& error, std::size_t bytes_transferred);
		void OnSendCompleted(const boost::system::error_code& error, const std::shared_ptr<Buffer>& buffer, std::size_t bytes_transferred);
		bool OnDataReceived(const PacketSegment& segment);

		void FlushSend();

		bool ReceiveImpl(const size_t length);
		void SendImpl(const BufferView& buffer);
	};
}
