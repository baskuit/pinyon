#pragma once

#include <libpinyon/packs.hh>

#include <tuple>

namespace detail
{

    template <
        template <typename, template <typename, typename, typename> typename, bool> typename TreeBanditTemplate,
        template <typename, typename, typename> typename NodeTypes,
        typename... BanditTypeList,
        typename... BanditTypes>
    auto algorithm_generator_unpack_bandits(
        std::tuple<BanditTypeList...> bandit_type_list,
        std::tuple<BanditTypes...> bandit_tuple)
    // -> std::tuple<typename TreeBanditTemplate<BanditTypeList, NodeTypes, true>::Search...>
    {
        return std::apply([](auto... pair)
                          { return std::tuple<typename TreeBanditTemplate<decltype(pair.first), NodeTypes, true>::Search...>(
                                typename TreeBanditTemplate<decltype(pair.first), NodeTypes, true>::Search(pair.second)...); },
                          zip(bandit_type_list, bandit_tuple));
    }

    template <
        template <typename, template <typename...> typename, bool> typename TreeBanditTemplate,
        typename BanditTuple,
        typename BanditTypePack,
        template <typename, typename, typename> typename... NodeTypes>
    auto algorithm_generator_unpack_nodes(BanditTypePack bandit_type_pack, BanditTuple bandit_tuple, NodePack<NodeTypes...> node_type_pack)
    {
        return std::tuple_cat((
            algorithm_generator_unpack_bandits<TreeBanditTemplate, NodeTypes>(bandit_type_pack, bandit_tuple))...);
    }
};

template <
    template <typename, template <typename...> typename, bool> typename... TreeBanditTemplates,
    typename BanditTypePack,
    typename BanditTuple,
    typename NodeTypePack>
auto algorithm_generator(BanditTypePack bandit_type_pack, BanditTuple bandit_tuple, NodeTypePack node_type_pack)
{
    return std::tuple_cat((
        detail::algorithm_generator_unpack_nodes<TreeBanditTemplates>(bandit_type_pack, bandit_tuple, node_type_pack))...);
}
/*

    e.g. algorithm_generator<TreeBandit, TreeBanditThreaded>(std::tuple<...> bandit_tuple, TypePack<DefaultNodes, LNodes>);

*/