#pragma once

#include <libpinyon/packs.hh>

#include <tuple>

/*

Utility to generate type lists for

TreeBanditTemplate <BanditType, NodeType>

given a list/pack of the 3 above

*/

namespace detail
{

    template <
        template <typename, template <typename, typename, typename> typename, bool> typename TreeBanditTemplate,
        template <typename, typename, typename> typename NodeTemplate,
        typename... BanditTypeList,
        typename... BanditTypes>
    auto algorithm_generator_unpack_bandits(
        std::tuple<BanditTypeList...> bandit_type_list,
        std::tuple<BanditTypes...> bandit_tuple)
    {
        return std::apply(
            [](auto... pair)
            { return std::tuple<typename TreeBanditTemplate<decltype(pair.first), NodeTemplate, true>::Search...>(
                //   typename TreeBanditTemplate<decltype(pair.first), NodeTemplate, true>::Search(pair.second)...); },
                  std::make_from_tuple<typename TreeBanditTemplate<decltype(pair.first), NodeTemplate, true>::Search>(pair.second)...); },
                  // i.e. TreeBandit<Types, NodeTemplate, true>::Search (&bandit)...
            zip(bandit_type_list, bandit_tuple));
    }

    template <
        template <typename, template <typename...> typename, bool> typename TreeBanditTemplate,
        typename BanditTuple,
        typename BanditTypePack,
        template <typename, typename, typename> typename... NodeTemplates>
    auto algorithm_generator_unpack_nodes(BanditTypePack bandit_type_pack, BanditTuple bandit_tuple, NodeTemplatePack<NodeTemplates...> node_type_pack)
    {
        return std::tuple_cat((
            algorithm_generator_unpack_bandits<TreeBanditTemplate, NodeTemplates>(bandit_type_pack, bandit_tuple))...);
    }
};

template <
    template <typename, template <typename...> typename, bool> typename... TreeBanditTemplates,
    typename BanditTypePack,
    typename BanditTuple,
    typename NodeTemplatePack>
auto algorithm_generator(BanditTypePack bandit_type_pack, BanditTuple bandit_tuple, NodeTemplatePack node_template_pack)
{
    return std::tuple_cat((
        detail::algorithm_generator_unpack_nodes<TreeBanditTemplates>(bandit_type_pack, bandit_tuple, node_template_pack))...);
}
/*

    e.g. algorithm_generator<TreeBandit, TreeBanditThreaded>
    (std::tuple<...> bandit_tuple, TypePack<DefaultNodes, LNodes>);

*/