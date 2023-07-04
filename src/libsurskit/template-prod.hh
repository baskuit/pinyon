#pragma once

#include <tuple>

/*
TODO rename probably and include the Ts so that T and Template<T> are associated
e.g. Algorithm and Model are paired for Algorithm::run() usage
*/

template <typename... Ts>
struct type_list
{
};

namespace detail
{

    template <template <typename> typename Template, typename... Ts, typename ArgTuple>
    auto cartesian_product_one(std::tuple<Ts...>, ArgTuple &&args) -> std::tuple<Template<Ts>...>
    {
        return {std::make_from_tuple<Template<Ts>>(args)...};
    }

    template <template <typename> typename Template, typename... Ts, typename ArgTuple>
    auto cartesian_product_one(type_list<Ts...>, ArgTuple &&args) -> std::tuple<Template<Ts>...>
    {
        return {std::make_from_tuple<Template<Ts>>(args)...};
    }

    template <template <typename> typename Template, typename... Ts>
    auto cartesian_product_one_(std::tuple<Ts...> &&args) -> std::tuple<Template<Ts>...>
    {
        return std::apply([](auto... elems)
                          { return std::make_tuple(Template<Ts>(elems)...); },
                          args);
    }

}

template <template <typename> typename... Templates, typename Tuple, typename... Args>
auto cartesian_product(Tuple tuple, Args &&...args)
{
    // std::tuple(arg) wraps each arg in a tuple, unless it already is a tuple -- problematic if you ever want a constructor that actually takes a tuple!
    return std::tuple_cat(detail::cartesian_product_one<Templates>(tuple, std::tuple(args))...);
}

template <template <typename> typename... Templates, typename Tuple>
auto cartesian_product(Tuple &&tuple)
{
    // std::tuple(arg) wraps each arg in a tuple, unless it already is a tuple -- problematic if you ever want a constructor that actually takes a tuple!
    return std::tuple_cat(detail::cartesian_product_one_<Templates>(std::move(tuple))...);
}