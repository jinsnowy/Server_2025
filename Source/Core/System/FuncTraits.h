#pragma once

namespace System {
    template <typename T, typename = void>
    struct FuncTraits {
    };

    template <typename R, typename... A>
    struct FuncTraits<R(A...)> {
        using ReturnType = R;
        using ClassType = void;
        using ArgsType = std::tuple<A...>;
        using FArgsType = std::tuple<A...>;
    };

    template <typename R, typename... A>
    struct FuncTraits<R(*)(A...)> {
        using ReturnType = R;
        using ClassType = void;
        using ArgsType = std::tuple<A...>;
        using FArgsType = std::tuple<A...>;
    };

    template <typename R, typename C, typename... A>
    struct FuncTraits<R(C::*)(A...)> {
        using ReturnType = R;
        using ClassType = C;
        using ArgsType = std::tuple<A...>;
        using FArgsType = std::tuple<C*, A...>;
    };

    template <typename R, typename C, typename... A>
    struct FuncTraits<R(C::*)(A...) const> {
        using ReturnType = R;
        using ClassType = C;
        using ArgsType = std::tuple<A...>;
        using FArgsType = std::tuple<const C*, A...>;
    };

    template <typename T>
    struct FuncTraits<T, std::void_t<decltype(&T::operator())>>
        : public FuncTraits<decltype(&T::operator())> { // for lambda
        using FArgsType = typename FuncTraits<decltype(&T::operator())>::ArgsType;
    };

	template <typename F>
	using FuncReturnT = typename FuncTraits<F>::ReturnType;
	
    template <typename F>
	using FuncClassT = typename FuncTraits<F>::ClassType;

	template <typename F>
	using FuncArgsT = typename FuncTraits<F>::ArgsType;

	template <typename F>
	using FuncFArgsT = typename FuncTraits<F>::FArgsType;

    template<typename T, size_t>
    struct TupleArg {
    };

    template<typename ...Args, size_t idx>
    struct TupleArg<std::tuple<Args...>, idx> {
        using Type = std::tuple_element_t<idx, std::tuple<Args...>>;
    };

    template<size_t idx>
    struct TupleArg<std::tuple<>, idx> {
        using Type = void;
    };

    template<typename F, size_t idx>
    struct FuncArg : TupleArg<typename FuncTraits<F>::ArgsType, idx> {
        using Type = typename TupleArg<typename FuncTraits<F>::ArgsType, idx>::Type;
    };

    template<typename F, size_t idx>
	using FuncArgT = typename FuncArg<F, idx>::Type;
}

