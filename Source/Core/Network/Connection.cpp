#include "stdafx.h"
#include "Connection.h"
#include "Core/Network/Session.h"
#include "Core/System/Scheduler.h"
#include "Core/System/Channel.h"
#include "Core/Network/Resolver.h"
#include "Core/Network/Socket.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"

namespace Network {
	Connection::Connection(std::unique_ptr<Socket> socket) 
		:
		System::Actor<Connection>(System::Channel(socket->context())),
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
		resolver_->Resolve(ip_, port_, &Connection::OnResolved, shared_from_this(), session);
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

	void Connection::OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results, std::shared_ptr<Session> session) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] resolve to {} failed by {}", ToString(), error.message());
			return;
		}

		socket_->ConnectAsync(results, &Connection::OnConnected, shared_from_this(), session);
	}

	void Connection::OnConnected(const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint& endpoint, std::shared_ptr<Session> session) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] connect to {} failed by {}", ToString(), error.to_string());
			return;
		}

		socket_->raw_socket().set_option(boost::asio::socket_base::keep_alive(true));
		socket_->raw_socket().set_option(boost::asio::socket_base::linger(true, 0));

		ip_ = endpoint.address().to_string();
		port_ = endpoint.port();
		session_ = session;

		send_stream_->is_sending = false;
		recv_stream_->buffer.Allocate(Buffer::kDefault);

		BeginReceive();

		session->Post([conn = shared_from_this()](Session& session) {
			session.set_connection(conn);
			session.OnConnected();
		});
	}

	void Connection::BeginReceive() {
		DEBUG_ASSERT(IsSynchronized());
		socket_->ReadAsync(recv_stream_->buffer.buffer_shared(), recv_stream_->buffer.size(), &Connection::OnReceived, shared_from_this());
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

	void Connection::OnSendCompleted(const boost::system::error_code& error, std::size_t bytes_transferred) {
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
		
		if (length < PacketHeader::Size()) {
			LOG_ERROR("[SESSION] OnReceived: invalid length is too small : {}", length);
			return false;
		}

		std::list<PacketSegment> completionPackets;

		const char* data = recv_stream_->buffer.data();
		uint32_t dataLength = static_cast<uint32_t>(length);
		uint32_t readOffset = 0;
		while (dataLength > 0) {
			const char* readDataPtr = data + readOffset;

			if (recv_stream_->pending.has_value()) {
				auto& pendingStream = recv_stream_->pending.value();
				uint32_t currentReadableSize = std::min(pendingStream.remainSegmentLength, dataLength);
				uint32_t currentPacketBufferOffset = pendingStream.packetBuffer.size() - pendingStream.remainSegmentLength;
				memcpy_s(pendingStream.packetBuffer.data() + currentPacketBufferOffset, pendingStream.remainSegmentLength, readDataPtr, currentReadableSize);
				pendingStream.remainSegmentLength -= currentReadableSize;
				if (pendingStream.remainSegmentLength == 0) {
					completionPackets.emplace_back(PacketSegment{ pendingStream.packetBuffer.data(), pendingStream.packetBuffer.size() });
					recv_stream_->pending.reset();
				}

				dataLength -= currentReadableSize;
				readOffset += currentReadableSize;
			}
			else {
				const PacketHeader* header = PacketHeader::Peek(readDataPtr);
				if (header->size >= PacketHeader::kMaxSize) {
					LOG_ERROR("[SESSION] OnReceived: invalid length is too large : {}", header->size);
					return false;
				}

				uint32_t packetReadableSize = PacketHeader::Size() + header->size;
				if (packetReadableSize > dataLength) {
					std::vector<char> packetBuffer(packetReadableSize);
					memcpy_s(packetBuffer.data(), packetReadableSize, readDataPtr, dataLength);

					recv_stream_->pending.emplace(PendingStream{
						.header = *header,
						.packetBuffer = std::move(packetBuffer),
						.remainSegmentLength = packetReadableSize - dataLength,
					});

					return true;
				}

				completionPackets.emplace_back(PacketSegment{ readDataPtr, packetReadableSize });

				dataLength -= packetReadableSize;
				readOffset += packetReadableSize;
			}
		}

		if (completionPackets.empty() == false) {
			session->Post([completionPackets = std::move(completionPackets)](Session& session) {
				for (const auto& completion_packet : completionPackets) {
					session.OnProcessPacket(completion_packet);
				}
			});
		}

		return true;
	}

	void Connection::Send(const Buffer& buffer) {
		DEBUG_ASSERT(IsSynchronized());
		if (buffer.IsEmpty()) {
			return;
		}

		if (buffer.GetByteCount() > PacketHeader::kMaxSize) {
			LOG_ERROR("[SESSION] Send: buffer size is too large : {}", buffer.GetByteCount());
			return;
		}

		send_stream_->buffers.push_back(buffer);
		if (send_stream_->is_sending == false) {
			FlushSend();
		}
	}

	std::string Connection::ToString() const {
		return FORMAT("{}:{}", ip_, port_);
	}

	void Connection::FlushSend(bool continueOnWriter) {
		DEBUG_ASSERT(IsSynchronized());
		if (continueOnWriter == false && send_stream_->is_sending == true) {
			return;
		}

		if (send_stream_->buffers.empty() || send_stream_->buffers.front().IsEmpty()) {
			send_stream_->is_sending = false;
			return;
		}

		auto& buffer = send_stream_->buffers.front();
		if (buffer.GetByteCount() < 0) {
			LOG_ERROR("[SESSION] FlushSend: invalid buffer size : {}", buffer.GetByteCount());
			Disconnect();
			return;
		}

		send_stream_->is_sending = true;
		SendImpl(buffer);

		buffer.set_start_pos(buffer.end_pos());
		if (buffer.GetRemainingByteCount() <= PacketHeader::Size()) {
			send_stream_->buffers.pop_front();
		}
	}

	void Connection::SendImpl(const Buffer& buffer) {
		DEBUG_ASSERT(IsSynchronized());
		socket_->WriteAsync(buffer.buffer_shared(), buffer.start_pos(), buffer.GetByteCount(), &Connection::OnSendCompleted, shared_from_this());
	}
}
