#pragma once

#include <tuple>

template<typename... Ts>
struct type_list {};

namespace detail {

template<template<typename> typename Template, typename... Ts, typename ArgTuple>
auto cartesian_product_one(type_list<Ts...>, ArgTuple&& args) -> std::tuple<Template<Ts>...> {
    return {std::make_from_tuple<Template<Ts>>(args)...};
}

}

template<template<typename> typename... Templates, typename TypeList, typename... Args>
auto cartesian_product(TypeList types, Args&&... args) {
    // std::tuple(arg) wraps each arg in a tuple, unless it already is a tuple -- problematic if you ever want a constructor that actually takes a tuple!
    return std::tuple_cat(detail::cartesian_product_one<Templates>(types, std::tuple(args))...);
}