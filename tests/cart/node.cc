#include <surskit.hh>

int main () {

    using Model = MonteCarloModel<MoldState<9>>;

    Exp3<Model> bandit{.01};

    std::tuple<Exp3<Model>, Exp3<Model>> bandit_tuple{bandit, bandit};
    TypePack<DefaultNodes, LNodes> node_type_pack{};

    auto x = algorithm_generator<TreeBandit>(bandit_tuple, node_type_pack);
    // auto count = std::tuple_size_v<decltype(x)>;
    // std::cout << count << std::endl;

    // auto y = std::get<0>(x);

    // MoldState<9> state{10};
    // Model model{0};
    // MatrixNode<TreeBandit<Exp3<Model>, NodeWrapper::MNode, NodeWrapper::CNode>>  root{}; 

    // prng device{0};

    // y.run(1000, device, state, model, root);


    return 0;
}