#pragma once

namespace Network {
	class StreamWriter;
	class OutputStream;
	class SendNode {
	public:
		virtual ~SendNode() = default;
		virtual bool SerializeTo(OutputStream& output_stream) = 0;
		virtual uint64_t GetMessageId() const = 0;
		virtual size_t GetSize() const = 0;
	};

	class RawNode : public SendNode {
	public:
		RawNode(uint64_t message_id, std::string&& data)
			:
			message_id_(message_id),
			data_(std::move(data)) {
		}

		RawNode(uint64_t message_id, const std::string& data)
			:
			message_id_(message_id),
			data_(data) {
		}

		bool SerializeTo(OutputStream& output_stream) override;

		uint64_t GetMessageId() const override {
			return message_id_;
		}
		
		size_t GetSize() const override {
			return data_.size();
		}

	private:
		uint64_t message_id_;
		std::string data_;
	};
}

