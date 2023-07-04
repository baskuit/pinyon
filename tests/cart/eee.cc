#include <surskit.hh>

template <class M>
class MatrixUCB {
public:
    MatrixUCB (double x) {
        std::cout << x << std::endl;
    }
    MatrixUCB(M m) {
        // std::cout << m.gamma << std::endl;
    }

    struct Types : M::Types {
        using Model = M;
    };
};

int main () {

    auto states =  cartesian_product
        <RandomTree, RandomTree>
            (type_list<SimpleTypes, RandomTreeTypes>(), 
        std::make_tuple(0, 1, 2, 2, 1, 0), std::make_tuple(0, 1, 2, 2, 1, 0));


    auto models = cartesian_product
        <MonteCarloModel, MonteCarloModel>
            (states,
            0, 1);

    auto bandit_algorithms = cartesian_product
        <Exp3, Exp3, Exp3, MatrixUCB, MatrixUCB, MatrixUCB>
            (models,
        .01, .1, 1, 1, 1.5, 2);

    auto tree_bandit_algorithms = cartesian_product
        <TreeBandit>
            (bandit_algorithms);

    auto count = std::tuple_size_v<decltype(tree_bandit_algorithms)>;

    std::cout << count << " algorithm sessions were generated!" << std::endl; 
}