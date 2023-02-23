#include "tree_state.hh"
#include "search/exp3p.hh"
#include "search/matrix_ucb.hh"

#include <iostream>

int main()
{

    const int size = 4;

    using TreeState = TreeState<size>;
    using MatrixUCB = MatrixUCB<MonteCarlo<TreeState>>;
    using Exp3p = Exp3p<MonteCarlo<TreeState>>;

     // Initialization now runs the Grow algorithm automatically
    prng device;
    TreeState tree_state(device, 3, 3, 3);
    MonteCarlo<TreeState> model(device);

    std::cout << "tree_state size: " << tree_state.count() << std::endl;
    std::cout << "tree_state expected value: " << std::endl;
    tree_state.get_expected_value_matrix().print();

    std::cout << "tree_state strategies: " << std::endl;
    for (int i = 0; i < tree_state.rows; ++i)
    {
        std::cout << tree_state.strategy0[i] << ' ';
    }
    std::cout << std::endl;
    for (int j = 0; j < tree_state.cols; ++j)
    {
        std::cout << tree_state.strategy1[j] << ' ';
    }
    std::cout << std::endl;

    // MatrixUCB search with multiple c_uct values.

    MatrixUCB matrix_ucb_session(device);
    Exp3p exp3p_session(device);

    

    double c_ucts[3] = {2, 1.4142, 1};
    for (int i = 0; i < 3; ++i)
    {
        double c_uct = c_ucts[i];
        matrix_ucb_session.c_uct = c_uct;
        matrix_ucb_session.require_interior = false;
        matrix_ucb_session.expl_threshold = .05;
        int playouts = 800;

        MatrixNode<Exp3p> matrix_ucb_root;
        exp3p_session.search(playouts, tree_state, model, matrix_ucb_root);
    }

    return 0;
}