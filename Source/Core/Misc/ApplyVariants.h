#pragma once

namespace Misc
{
	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	template<typename... Types, typename... Funcs>
	static auto Apply(std::variant<Types...>& v, Funcs&&... funcs) {
		return std::visit(overloaded{ std::forward<Funcs>(funcs)... }, v);
	}

	template<typename... Types, typename... Funcs>
	static auto Apply(const std::variant<Types...>& v, Funcs&&... funcs) {
		return std::visit(overloaded{ std::forward<Funcs>(funcs)... }, v);
	}
}