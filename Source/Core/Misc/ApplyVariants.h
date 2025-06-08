#pragma once

namespace Misc {
	namespace Detail {
		template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
		template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;
	}

	template<typename... Types, typename... Funcs>
	static auto Apply(std::variant<Types...>& v, Funcs&&... funcs) {
		return std::visit(Detail::Overloaded{ std::forward<Funcs>(funcs)... }, v);
	}

	template<typename... Types, typename... Funcs>
	static auto Apply(const std::variant<Types...>& v, Funcs&&... funcs) {
		return std::visit(Detail::Overloaded{ std::forward<Funcs>(funcs)... }, v);
	}
}