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

namespace Network {
	Connection::Connection(std::unique_ptr<Socket> socket) 
		:
		System::Actor(System::Channel(socket->context())),
		socket_(std::move(socket)),
		resolver_(std::make_unique<Resolver>(socket_->context())),
		send_stream_(std::make_unique<SendNetworkStream>()),
		recv_stream_(std::make_unique<RecvNetworkStream>())
	{
	}

	Connection::Connection(std::shared_ptr<System::Context> context)
		:
		socket_(std::make_unique<Socket>(context)),
		resolver_(std::make_unique<Resolver>(context)),
		send_stream_(std::make_unique<SendNetworkStream>()),
		recv_stream_(std::make_unique<RecvNetworkStream>()) {
	}

	Connection::~Connection() {
		if (IsConnected()) {
			Disconnect();
		}
	}

	void Connection::Connect(const std::string& ip, const uint16_t& port, std::shared_ptr<Session> session) {
		DEBUG_ASSERT(IsSynchronized());

		ip_ = ip;
		port_ = port;
		session_ = session;
		resolver_->Resolve(ip_, port_, &Connection::OnResolved, GetShared(this));
	}

	void Connection::Disconnect() {
		DEBUG_ASSERT(IsSynchronized());
		if (socket_->IsOpen()){
			socket_->Close();
		}

		auto session = session_.lock();
		if (session) {
			session->OnDisconnected();
			session_.reset();
		}
	}

	bool Connection::IsConnected() const {
		return socket_->IsOpen();
	}

	void Connection::Send(const BufferView& buffer) {
		DEBUG_ASSERT(IsSynchronized());
		send_stream_->pending_buffers.push_back(buffer);
		if (send_stream_->is_sending == false) {
			FlushSend(false);
		}
	}

	bool Connection::IsSendInProgress() const {
		return send_stream_->is_sending.load();
	}

	void Connection::OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] resolve to {} failed by {}", ToString(), error.message());
			return;
		}

		socket_->ConnectAsync(results, &Connection::OnConnected, GetShared(this));
	}

	void Connection::OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] connect to {} failed by {}", ToString(), error.to_string());
			return;
		}

		socket_->raw_socket().set_option(boost::asio::socket_base::keep_alive(true));
		socket_->raw_socket().set_option(boost::asio::socket_base::linger(true, 0));

		ip_ = endpoint.address().to_string();
		port_ = endpoint.port();

		send_stream_->is_sending = false;
		recv_stream_->buffer.reset(new Buffer(RequestRecvBuffer()));

		auto session = session_.lock();
		if (session == nullptr) {
			LOG_ERROR("[CONNECTION] session is null after connection established");
			return;
		}

		BeginReceive();

		protocol_ = session->CreateProtocol();

		Ctrl(*session).Post([conn = GetShared(this)](Session& session) {
			session.set_connection(conn);
			session.OnConnected();
		});
	}

	void Connection::BeginReceive() {
		DEBUG_ASSERT(IsSynchronized());
		socket_->ReadAsync(*recv_stream_->buffer, &Connection::OnReceived, GetShared(this));
	}

	void Connection::OnReceived(const boost::system::error_code& error, std::size_t bytes_transferred) {
		DEBUG_ASSERT(IsSynchronized());

		if (socket_->IsOpen() == false) {
			Disconnect();
			return;
		}

		if (bytes_transferred == 0) {
			Disconnect();
			return;
		}

		if (error) {
			Disconnect();
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

	void Connection::OnSendCompleted(const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("Error during async_write: {}", error.message());
		}

		auto session = session_.lock();
		if (session == nullptr) {
			return;
		}

		FlushSend(true);
	}

	bool Connection::ReceiveImpl(const size_t length) {
		auto session = session_.lock();
		if (session == nullptr) {
			return false;
		}

		if (protocol_ == nullptr) {
			return false;
		}
		
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
					PacketSegment completion_packet{
						.data = pending_stream.dataBuffer.data(),
						.length = pending_stream.dataBuffer.size()
					};

					if (protocol_->ProcessReceiveData(completion_packet) == false) {
						LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", completion_packet.header().id);
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

				if (protocol_->ProcessReceiveData(completion_packet) == false) {
					LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", completion_packet.header().id);
				}
				else {
					has_any_completion_packets = true;
				}

				processing_data_size += completion_packet.length;
			}
		}

		if (has_any_completion_packets == true) {
			Ctrl(*session).Post([protocol = protocol_](Session& session) {
				session.OnProcessPacket(protocol);
			});
		}

		return true;
	}

	std::string Connection::ToString() const {
		return FORMAT("{}:{}", ip_, port_);
	}

	void Connection::FlushSend(bool continueOnWriter) {
		DEBUG_ASSERT(IsSynchronized());
		if (continueOnWriter == false && send_stream_->is_sending == true) {
			return;
		}

		if (send_stream_->pending_buffers.empty()) {
			send_stream_->is_sending = false;
			auto session = session_.lock();
			if (session != nullptr) {
				Ctrl(*session).Post([](Session& session) {
					session.FlushToSendStream();
				});
			}
			return;
		}

		send_stream_->is_sending = true;
		SendImpl(send_stream_->pending_buffers.front());
		send_stream_->pending_buffers.pop_front();
	}

	void Connection::SendImpl(const BufferView& buffer) {
		DEBUG_ASSERT(IsSynchronized());
		socket_->WriteAsync(buffer, &Connection::OnSendCompleted, GetShared(this));
	}
}
