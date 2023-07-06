#include <surskit.hh>

#include <tuple>

struct NodeWrapper {

    template <typename BanditAlgorithm>
    struct MNode : MatrixNode<BanditAlgorithm> {

    };

    template <typename BanditAlgorithm>
    struct CNode : ChanceNode<BanditAlgorithm> {

    };

};

template <template <typename> typename... Template>
struct TemplatePack {};

template <typename... Type>
struct TypePack {};

template <
    template <typename, template <typename> typename, template <typename> typename> typename TreeBanditTemplate, 
    typename NodeTypes, 
    typename... BanditTypes>
auto buzz (std::tuple<BanditTypes...> bandit_tuple) -> 
        std::tuple<TreeBanditTemplate<BanditTypes, typename NodeTypes::template MNode, typename NodeTypes::template CNode>...>{
    return {std::make_from_tuple<TreeBanditTemplate<BanditTypes, typename NodeTypes::template MNode, typename NodeTypes::template CNode>>(bandit_tuple)...};
};

template <
    template <typename, template <typename> typename, template <typename> typename> typename TreeBanditTemplate, 
    typename BanditTuple, 
    typename... NodeTypes>
auto bar (BanditTuple bandit_tuple, TypePack<NodeTypes...> node_type_pack) {
    return std::tuple_cat((
        buzz<TreeBanditTemplate, NodeTypes>(bandit_tuple)
    )...);
}


template <
    template <typename, template <typename> typename, template <typename> typename> typename... TreeBanditTemplates, 
    typename BanditTuple, 
    typename NodeTypePack>
auto foo (BanditTuple bandit_tuple, NodeTypePack node_type_pack) {
    return std::tuple_cat((
        bar<TreeBanditTemplates>(bandit_tuple, node_type_pack)
    )...);
}

int main () {

    using Model = MonteCarloModel<MoldState<9>>;

    Exp3<Model> bandit{.01};

    std::tuple<Exp3<Model>> bandit_tuple{bandit};
    TypePack<NodeWrapper> node_type_pack{};

    auto x = foo<TreeBandit>(bandit_tuple, node_type_pack);

    return 0;
}