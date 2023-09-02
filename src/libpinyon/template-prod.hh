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

template <template <typename> typename... Ts>
struct template_list
{
};

namespace detail
{

    template <template <typename> typename Template, typename... ParamTs, typename ArgTuple>
    auto cartesian_product_per_template_helper(type_list<ParamTs...>, ArgTuple &&arg_tuple) -> std::tuple<Template<ParamTs>...>
    {
        return {std::make_from_tuple<Template<ParamTs>>(arg_tuple)...};
    }

    // same as above, but uses tuple for param_pack
    template <template <typename> typename Template, typename... ParamTs, typename ArgTuple>
    auto cartesian_product_per_template_helper(std::tuple<ParamTs...>, ArgTuple &&arg_tuple) -> std::tuple<Template<ParamTs>...>
    {
        return {std::make_from_tuple<Template<ParamTs>>(arg_tuple)...};
    }

    template <typename ParamT, template <typename> typename... Templates, typename ArgTuple>
    auto cartesian_product_per_param_helper(template_list<Templates...>, ArgTuple &&arg_tuple) -> std::tuple<Templates<ParamT>...>
    {
        return {std::make_from_tuple<Templates<ParamT>>(arg_tuple)...};
        // return std::apply([](auto... elems)
        //                   { return std::make_tuple(Template<Ts>(elems)...); },
        //                   args);
    }

    template <template <typename...> typename Template, typename... TemplateParams>
    auto type_list_cart_prod_helper (type_list<TemplateParams...>) -> std::tuple<Template<TemplateParams>...>
    {
        return {Template<TemplateParams>{}...};
    }

}

template <template <typename> typename... Templates, typename ParamPack, typename... Args>
auto cartesian_product_per_template(ParamPack param_pack, Args &&...args_per_template)
{
    // returns a tuple of 
    // TemplatesFirst<ParamPackFirst>{args_first}, ..., TemplatesFirst<ParamPackLast>{args_first}, ... 
    // ...
    // TemplateLast<ParamPackFirst>(args_last), ..., TemplateLast<ParamPackLast>(args_last)
    return std::tuple_cat(detail::cartesian_product_per_template_helper<Templates>(param_pack, std::tuple(args_per_template))...);
}

template <typename... Params, typename TemplatePack, typename... Args>
auto cartesian_product_per_param(TemplatePack template_pack, Args &&...args_per_param)
{
    // returns a tuple of 
    // TemplatesFirst<ParamPackFirst>{args_first}, ..., TemplatesFirst<ParamPackLast>{args_last}, ... 
    // ...
    // TemplateLast<ParamPackFirst>(args_first), ..., TemplateLast<ParamPackLast>(args_last)
    return std::tuple_cat(detail::cartesian_product_per_param_helper<Params>(template_pack, std::tuple(args_per_param))...);
}

template <typename... Params, typename TemplatePack, typename... Args>
auto cartesian_product_per_param(TemplatePack template_pack, std::tuple<Args...>& args_per_param)
{
    return std::tuple_cat(detail::cartesian_product_per_param_helper<Params>(template_pack, std::make_tuple(args_per_param))...);
}

template <typename TemplatePack, typename... Args>
auto cartesian_product_per_param(TemplatePack template_pack, std::tuple<Args...>& args_per_param)
{
    return std::tuple_cat(detail::cartesian_product_per_param_helper<Args>(template_pack, std::make_tuple(args_per_param))...);
}

/*

New

*/

template <template <typename...> typename... Templates, typename TemplateParams>
auto type_list_cart_prod (TemplateParams template_params)
{
    return std::tuple_cat(
        detail::type_list_cart_prod_helper<Templates>(template_params)...
    );
}

template <template <typename...> typename... Templates, typename TemplateParams, typename T>
auto type_list_cart_prod_named (TemplateParams template_params)
{
    return std::tuple_cat(
        detail::type_list_cart_prod_helper<Templates>(template_params)...
    );
}