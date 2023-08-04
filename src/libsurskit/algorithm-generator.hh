#pragma once

#include <libsurskit/packs.hh>

#include <tuple>

namespace detail
{

    template <
        template <typename, template <typename> typename, template <typename> typename> typename TreeBanditTemplate,
        typename NodeTypes,
        typename... BanditTypes>
    auto algorithm_generator_unpack_bandits(std::tuple<BanditTypes...> bandit_tuple) -> std::tuple<TreeBanditTemplate<BanditTypes, NodeTypes::template MNode, NodeTypes::template CNode>...>
    {
        return std::apply([](auto... bandits)
                          { return std::tuple<TreeBanditTemplate<BanditTypes, NodeTypes::template MNode, NodeTypes::template CNode>...>(
                                TreeBanditTemplate<BanditTypes, NodeTypes::template MNode, NodeTypes::template CNode>(bandits)...); },
                          bandit_tuple);
    }

    template <
        template <typename, template <typename> typename, template <typename> typename> typename TreeBanditTemplate,
        typename BanditTuple,
        typename... NodeTypes>
    auto algorithm_generator_unpack_nodes(BanditTuple bandit_tuple, TypePack<NodeTypes...> node_type_pack)
    {
        return std::tuple_cat((
            algorithm_generator_unpack_bandits<TreeBanditTemplate, NodeTypes>(bandit_tuple))...);
    }

};

template <
    template <typename, template <typename> typename, template <typename> typename> typename... TreeBanditTemplates,
    typename BanditTuple,
    typename NodeTypePack>
auto algorithm_generator(BanditTuple bandit_tuple, NodeTypePack node_type_pack)
{
    /*
    
        e.g. algorithm_generator<TreeBandit, TreeBanditThreaded>(std::tuple<...> bandit_tuple, TypePack<DefaultNodes, LNodes>);
    
    */

    return std::tuple_cat((
        detail::algorithm_generator_unpack_nodes<TreeBanditTemplates>(bandit_tuple, node_type_pack))...);
}
