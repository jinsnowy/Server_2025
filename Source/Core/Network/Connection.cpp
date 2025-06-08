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
		resolver_->Resolve(ip_, port_, &Connection::OnResolved, GetShared(this), session);
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

	void Connection::OnResolved(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type results, std::shared_ptr<Session> session) {
		DEBUG_ASSERT(IsSynchronized());
		if (error) {
			LOG_ERROR("[CONNECTION] resolve to {} failed by {}", ToString(), error.message());
			return;
		}

		socket_->ConnectAsync(results, &Connection::OnConnected, GetShared(this), session);
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
		protocol_ = session->CreateProtocol();

		send_stream_->is_sending = false;
		recv_stream_->buffer.reset(new Buffer(RequestRecvBuffer()));

		BeginReceive();

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

		std::list<PacketSegment> completionPackets;

		const char* data = recv_stream_->buffer->GetBufferPtr();
		size_t dataLength = length;
		size_t readOffset = 0;
		while (dataLength > 0) {
			const char* readDataPtr = data + readOffset;
			if (recv_stream_->pending.has_value())  {
				auto& pendingStream = recv_stream_->pending.value();
				size_t currentReadableSize = std::min(pendingStream.remainSegmentLength, dataLength);
				size_t currentPacketBufferOffset = pendingStream.packetBuffer.size() - pendingStream.remainSegmentLength;
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
				if (dataLength < PacketHeader::Size()) {
					LOG_ERROR("[SESSION] OnReceived: invalid length is too small : {}", dataLength);
					return false;
				}

				const PacketHeader* header = PacketHeader::Peek(readDataPtr);
				if (header->size >= PacketHeader::kMaxSize) {
					LOG_ERROR("[SESSION] OnReceived: invalid length is too large : {}", header->size);
					return false;
				}

				size_t packetReadableSize = PacketHeader::Size() + header->size;
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
			for (const auto& completion_packet : completionPackets) {
				if (protocol_->ProcessReceiveData(completion_packet) == false) {
					LOG_ERROR("[SESSION] OnProcessPacket: invalid packet id : {}", completion_packet.header().id);
					return false;
				}
			}
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
