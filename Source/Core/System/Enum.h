#pragma once

#include "Core/ThirdParty/MagicEnum/magic_enum.hpp"

namespace System {
	class Enums {
	public:
		template<typename E>
		static std::string_view ToString(const E& value) {
			return magic_enum::enum_name(value);
		}

		template<typename E>
		static auto Value(int32_t index) {
			return magic_enum::enum_value<E>(index);
		}

		template<typename E>
		static auto Values() {
			return magic_enum::enum_values<E>();
		}

		template<typename E>
		static auto Name(int32_t index) {
			return magic_enum::enum_name<E>(Value<E>(index));
		}

		template<typename E>
		static auto Names() {
			return magic_enum::enum_names<E>();
		}

		template<typename E, typename underlying_type = std::underlying_type_t<std::decay_t<E>>>
		static E Cast(underlying_type value) {
			return magic_enum::enum_cast<E>(value);
		}

		template<typename E>
		static constexpr size_t Count() {
			return magic_enum::enum_count<E>();
		}
	};
}