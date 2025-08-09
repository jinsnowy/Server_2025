#pragma once

namespace System {
namespace Detail {
	class IMessage {
	public:
		virtual ~IMessage() = default;
		virtual void Execute() = 0;
		virtual const void* signature() const = 0;
	};
} // namepspace Detail
} // namespace System


