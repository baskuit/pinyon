#include "tree_state.hh"
#include "search/exp3p.hh"
#include "search/matrix_ucb.hh"

#include <iostream>

int main () {
    prng device;

    using TreeState = TreeState<4>;
    using MatrixUCB = MatrixUCB<MonteCarlo<TreeState>>;
    using Exp3p = Exp3p<MonteCarlo<TreeState>>;

    MatrixNode<Grow<MonteCarlo<SeedState<4>>>> root;

    TreeState tree(device, root, 6, 3, 3);
    std::cout << "tree size: " << tree.root.count() << std::endl;
    std::cout << "tree expected value: " << std::endl;
    tree.root.stats.expected_value.print();

    MatrixUCB matrix_ucb_session(device);
    Exp3p exp3p_session(device);

    MonteCarlo<TreeState> model(device);


    double c_ucts[3] = {10, 5, 1};
    for (int i = 0; i < 3; ++i) {
        double c_uct = c_ucts[i];
        matrix_ucb_session.c_uct = c_uct;
        matrix_ucb_session.expl_threshold = .005;
        MatrixNode<MatrixUCB> matrix_ucb_root;
        matrix_ucb_session.search(10000, tree, model, matrix_ucb_root);
    }   

    return 0;
}