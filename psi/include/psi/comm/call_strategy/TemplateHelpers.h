
#pragma once

#include <tuple>
#include <typeindex>
#include <typeinfo>

template <typename Func, size_t... N>
constexpr auto index_apply_impl(Func &&fn, std::index_sequence<N...>)
{
    return fn(std::integral_constant<size_t, N> {}...);
}

template <size_t N, typename Func>
constexpr auto index_apply(Func &&fn)
{
    return index_apply_impl(std::forward<Func>(fn), std::make_index_sequence<N> {});
}

template <typename...>
struct TypeList;

template <typename... Args>
struct VariadicCaller {
    template <typename Func>
    static void invoke(Func &&fn, std::tuple<Args...> &result)
    {
        return index_apply<sizeof...(Args)>(
            [&result, fn = std::forward<Func>(fn)](auto... N) { fn(std::get<N>(result)...); });
    }

    template <typename Func>
    static void invokeForEach(Func &&fn, const Args &...args)
    {
        auto x = {fn(args)...};
    }

    template <typename C, typename Func>
    static void invoke(C &&caller, Func &&fn, std::tuple<Args...> &result)
    {
        return index_apply<sizeof...(Args)>([caller = std::forward<C>(caller), fn = std::forward<Func>(fn), &result](
                                                auto... N) { (caller->*fn)(std::get<N>(result)...); });
    }

    /*template <typename Func>
    static auto invoke_C17(Func&& fn, Args&&... args)
    {
        return [fn = std::forward<Func>(fn), args = std::make_tuple(std::forward<Args>(args)...)]() mutable 
        {
            return std::apply([fn = std::forward<Func>(fn)](auto&&... args) mutable
            {
                fn(std::forward<Args>(args)...);
            }, std::move(args));
        };
    }

    template <typename Func>
    static auto invoke_C17(Func&& fn, Args... args)
    {
        return [fn = std::forward<Func>(fn), args = std::make_tuple(std::forward<Args>(args)...)]() mutable
        {
            return std::apply([fn = std::forward<Func>(fn)](auto&&... args) mutable
            {
                fn(std::forward<Args>(args)...);
            }, std::move(args));
        };
    }*/
};
