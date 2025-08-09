#include "stdafx.h"
#include "Connection.h"
#include "Core/Network/Session.h"
#include "Core/System/Scheduler.h"
#include "Core/System/Channel.h"
#include "Core/Network/Resolver.h"
#include "Core/Network/Socket.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/BufferPool.h"
#include "Core/Network/NetworkStream.h"
#include "Core/Network/Protocol.h"
#include "Core/Network/SendNode.h"

namespace Network {
	Connection::Connection(std::unique_ptr<Socket> socket) 
		:
		socket_(std::move(socket)),
		resolver_(std::make_unique<Resolver>(socket_->context())),
		output_stream_(std::make_unique<OutputStream>()),
		send_stream_(std::make_unique<SendNetworkStream>()),
		recv_stream_(std::make_unique<RecvNetworkStream>()) {
		socket_->raw_socket().set_option(boost::asio::socket_base::keep_alive(true));
		socket_->raw_socket().set_option(boost::asio::socket_base::linger(true, 0));
		const auto& remote_endpoint = socket_->raw_socket().remote_endpoint();
		connceted_address_ = IPAddress(remote_endpoint.address().to_string(), remote_endpoint.port());
	}

	Connection::Connection(std::shared_ptr<System::Context> context)
		:
		socket_(std::make_unique<Socket>(context)),
		resolver_(std::make_unique<Resolver>(context)),
		output_stream_(std::make_unique<OutputStream>()),
		send_stream_(std::make_unique<SendNetworkStream>()),
		recv_stream_(std::make_unique<RecvNetworkStream>()) {
		socket_->raw_socket().set_option(boost::asio::socket_base::keep_alive(true));
		socket_->raw_socket().set_option(boost::asio::socket_base::linger(true, 0));
	}

	Connection::~Connection() = default;

	void Connection::Connect(const std::string& ip, const uint16_t& port) {
		DEBUG_ASSERT(IsSynchronized());

		target_address_ = IPAddress(ip, port);
		resolver_->Resolve(ip, port, &Connection::OnResolved, SharedFrom(this));
	}

	void Connection::Disconnect() {
		if (socket_->IsOpen()){
			socket_->Close();
		}
	}

	bool Connection::IsConnected() const {
		return socket_->IsOpen();
	}

	void Connection::Send(std::unique_ptr<SendNode> node) {
		static constexpr size_t kMaxPendingBuffers = 512;

		send_stream_->pending_buffers.Push(std::move(node));
		if (send_stream_->pending_buffers.IncreaseNodeCount(1) == 0) {
			Ctrl(*this).Post([](Connection& connection) {
				connection.FlushSend();
			});
		}
		/*else if (pending_buffers_size > 1024) {
			LOG_ERROR("[CONNECTION] Send: too many pending buffers: {}", pending_buffers_size);
			Disconnect();
		}*/
	}

	void Connection::OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			on_connect_failed_delegate_.ExecuteIfBound(target_address_, error.message());
			return;
		}

		socket_->ConnectAsync(results, &Connection::OnConnected, SharedFrom(this));
	}

	void Connection::OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
		DEBUG_ASSERT(IsSynchronized());

		if (error) {
			on_connect_failed_delegate_.ExecuteIfBound(connceted_address_, error.message());
			return;
		}

		connceted_address_ = IPAddress(endpoint.address().to_string(), endpoint.port());

		BeginConnection();
	}

	void Connection::OnDisconnected() {
		DEBUG_ASSERT(IsSynchronized());
		on_disconnected_delegate_.ExecuteIfBound();
		socket_->Close();
	}

	void Connection::BeginConnection() {
		DEBUG_ASSERT(IsSynchronized());

		is_sending_ = false;
		output_stream_->Clear();
		send_stream_.reset(new SendNetworkStream());
		recv_stream_.reset(new RecvNetworkStream());

		if (protocol_factory_) {
			protocol_ = protocol_factory_();
		}
		else {
			LOG_WARNING("[Connection] Protocol factory is not set, using default protocol");
		}

		BeginReceive();

		on_connected_delegate_.ExecuteIfBound(SharedFrom(this), connceted_address_);

		LOG_INFO("[Connection] Begin Connection");
	}

	void Connection::BeginReceive() {
		DEBUG_ASSERT(IsSynchronized());
		socket_->ReadAsync(*recv_stream_->buffer, &Connection::OnReceived, SharedFrom(this));
	}

	void Connection::OnReceived(const boost::system::error_code& error, std::size_t bytes_transferred) {
		DEBUG_ASSERT(IsSynchronized());

		if (bytes_transferred == 0) {
			OnDisconnected();
			return;
		}

		if (error) {
			OnDisconnected();
			LOG_ERROR("Error during async_read: error_code: {}, message: {}", error.value(), error.message());
			return;
		}

		try {
			if (ReceiveImpl(bytes_transferred) == false) {
				LOG_ERROR("[CONNECTION] OnReceived failed");
				Disconnect();
				return;
			}

			BeginReceive();
		}
		catch (const std::exception& e) {
			LOG_ERROR("Connection::Receive() error: {}", e.what());
			Disconnect();
		}
	}

	bool Connection::ReceiveImpl(const size_t length) {
		if (recv_stream_->buffer->GetBufferSize() < length) {
			LOG_ERROR("[SESSION] OnReceived: invalid length is too large : {}", length);
			return false;
		}

		static_assert(sizeof(PacketHeader) == 16, "PacketHeader size must be 16 bytes");

		bool has_any_completion_packets = false;
		
		const char* buffer_ptr = recv_stream_->buffer->GetBufferPtr();
		size_t data_size = length;
		size_t processing_data_size = 0;
		while (processing_data_size < data_size) {
			const char* data_ptr = buffer_ptr + processing_data_size;

			// Check if we have a pending stream
			if (recv_stream_->pending.has_value()) {
				Network::PendingStream& pending_stream = recv_stream_->pending.value();

				if (pending_stream.dataBuffer.size() < sizeof(PacketHeader)) {
					size_t remaining_data_size = data_size - processing_data_size;
					size_t append_data_size = std::min(remaining_data_size, sizeof(PacketHeader) - pending_stream.dataBuffer.size());

					pending_stream.AppendData(data_ptr, append_data_size);

					data_ptr += append_data_size;
					processing_data_size += append_data_size;
					if (pending_stream.dataBuffer.size() < sizeof(PacketHeader)) {
						break;
					}

					// Now we have enough data to read the header
					std::memcpy(&pending_stream.header, pending_stream.dataBuffer.data(), sizeof(PacketHeader));
				}

				const PacketHeader& header = pending_stream.header;

				size_t remaining_data_size = data_size - processing_data_size;
				size_t required_data_size_to_complete = header.size + PacketHeader::Size();
				DEBUG_ASSERT(required_data_size_to_complete >= PacketHeader::Size());
				DEBUG_ASSERT(required_data_size_to_complete >= pending_stream.dataBuffer.size());
				if (required_data_size_to_complete < pending_stream.dataBuffer.size()) {
					LOG_ERROR("[SESSION] OnReceived: invalid packet size : {}", required_data_size_to_complete);
					return false;
				}

				size_t append_data_size = std::min(remaining_data_size, required_data_size_to_complete - pending_stream.dataBuffer.size());
				pending_stream.AppendData(data_ptr, append_data_size);
				data_ptr += append_data_size;
				processing_data_size += append_data_size;
				if (required_data_size_to_complete == pending_stream.dataBuffer.size())
				{
					const PacketSegment completion_packet{
						.data = pending_stream.dataBuffer.data(),
						.length = pending_stream.dataBuffer.size()
					};

					if (OnDataReceived(completion_packet) == false) {
						LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", completion_packet.header().id);
						return false;
					}
					else {
						has_any_completion_packets = true;
					}

					recv_stream_->pending.reset(); // Clear pending stream
				}
			}
			else {
				size_t remaining_data_size = data_size - processing_data_size;
				if (remaining_data_size < sizeof(PacketHeader)) {
					recv_stream_->pending.emplace(PendingStream{
						.header = PacketHeader{},
						.dataBuffer = {},
					}).AppendData(data_ptr, remaining_data_size);
					break; // wait for completion of the pending stream with more data
				}

				const PacketHeader& header = *PacketHeader::Peek(data_ptr);

				if (remaining_data_size < header.size + PacketHeader::Size()) {
					recv_stream_->pending.emplace(PendingStream{
						.header = header,
						.dataBuffer = {},
					}).AppendData(data_ptr, remaining_data_size);
					break; // wait for completion of the pending stream with more data
				}

				const PacketSegment completion_packet{
					.data = data_ptr ,
					.length = header.size + PacketHeader::Size(),
				};

				if (OnDataReceived(completion_packet)) {
					has_any_completion_packets = true;
				}
				else {
					LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", completion_packet.header().id);
				}

				processing_data_size += completion_packet.length;
			}
		}

		if (has_any_completion_packets == true) {
			on_data_received_delegate_.ExecuteIfBound();
		}

		return true;
	}

	bool Connection::OnDataReceived(const PacketSegment& segment) {
		if (protocol_ == nullptr) {
			LOG_WARNING("[CONNECTION] OnDataReceived: protocol is not set, skipping processing for message_id: {}", segment.header().id);
			return true;
		}
		return protocol_->ProcessReceiveData(segment);
	}

	std::string Connection::ToString() const {
		return connceted_address_.IsValid() ? connceted_address_.ToString() : target_address_.ToString();
	}

	void Connection::FlushSend() {
		DEBUG_ASSERT(IsSynchronized());

		std::unique_ptr<SendNode> node;
		size_t node_count = 0;
		while (send_stream_->pending_buffers.TryPop(node)) {
			++node_count;
			if (node->SerializeTo(*output_stream_) == false) {
				LOG_ERROR("[CONNECTION] FlushSend: failed to serialize node, message_id: {}", node->GetMessageId());
				continue;
			}
		}
		send_stream_->pending_buffers.DecreaseNodeCount(node_count);

		if (is_sending_ == true) {
			return;
		}

		auto buffer_view = output_stream_->NextBuffer();
		if (buffer_view.has_value()) {
			is_sending_ = true;
			SendImpl(buffer_view.value());
		}
		else if (node_count > 0) {
			RELEASE_ASSERT(false && "this is not expecting serialization behavior");
		}
	}

	void Connection::SendImpl(const BufferView& buffer) {
		DEBUG_ASSERT(IsSynchronized());
		socket_->WriteAsync(buffer, &Connection::OnSendCompleted, SharedFrom(this));
	}

	void Connection::OnSendCompleted(const boost::system::error_code& error, const std::shared_ptr<Buffer>& buffer, std::size_t bytes_transferred) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("Error during async_write: {}", error.message());
			Disconnect();
			return;
		}

		buffer->set_start_pos(buffer->start_pos() + bytes_transferred);

		auto buffer_view = output_stream_->NextBuffer();
		if (buffer_view.has_value()) {
			SendImpl(buffer_view.value());
		}
		else {
			is_sending_ = false; // Reset sending state
			FlushSend();
		}
	}
}
