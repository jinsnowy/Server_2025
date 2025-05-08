#pragma once

namespace Network {
	struct SendNetworkStream;
	class OutputStream {
	public:
		OutputStream(SendNetworkStream& send_stream);

		bool Next(void** data, int32_t* size);
		void BackUp(int32_t count);
		int64_t ByteCount() const;
		bool WriteAliasedRaw(const void* data, int32_t size);
		bool AllowsAliasing() const;

	private:
		SendNetworkStream* send_stream_;
	};
}

