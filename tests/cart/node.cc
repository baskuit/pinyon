#include <surskit.hh>

#include <tuple>

template <template <typename> typename... Template>
struct TemplatePack {};

template <typename... Type>
struct TypePack {};

struct NodeWrapper {

    template <typename BanditAlgorithm>
    using MNode = MatrixNode<BanditAlgorithm>;

    template <typename BanditAlgorithm>
    using CNode = ChanceNode<BanditAlgorithm>;
};

struct NodeWrapper2 {

    template <typename BanditAlgorithm>
    using MNode = MatrixNodeL<BanditAlgorithm>;

    template <typename BanditAlgorithm>
    using CNode = ChanceNodeL<BanditAlgorithm>;
};


template <
    template <typename, template <typename> typename, template <typename> typename> typename TreeBanditTemplate,
    typename NodeTypes,
    typename... BanditTypes>
auto buzz(std::tuple<BanditTypes...> bandit_tuple) -> std::tuple<TreeBanditTemplate<BanditTypes, NodeTypes::template MNode, NodeTypes::template CNode>...>
{
    return std::apply([](auto... bandits) {
        return std::tuple<TreeBanditTemplate<BanditTypes, NodeTypes::template MNode, NodeTypes::template CNode>...>(
            TreeBanditTemplate<BanditTypes, NodeTypes::template MNode, NodeTypes::template CNode>(bandits)...
        );
    }, bandit_tuple);
}

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

    std::tuple<Exp3<Model>, Exp3<Model>> bandit_tuple{bandit, bandit};
    TypePack<NodeWrapper, NodeWrapper2> node_type_pack{};

    auto x = foo<TreeBandit>(bandit_tuple, node_type_pack);
    auto count = std::tuple_size_v<decltype(x)>;
    std::cout << count << std::endl;
    
    // auto y = std::get<0>(x);

    // MoldState<9> state{10};
    // Model model{0};
    // MatrixNode<TreeBandit<Exp3<Model>, NodeWrapper::MNode, NodeWrapper::CNode>>  root{}; 

    // prng device{0};

    // y.run(1000, device, state, model, root);


    return 0;
}