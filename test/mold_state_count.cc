
#include <assert.h>
#include <iostream>

#include "state/test_states.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix_ucb.hh"
#include "tree/tree.hh"

#include <assert.h>

/*
Although templated, this currently only works with Bandit algorithms that intialize with just a prng device.
*/

template <
    template <class Model, template <class _Model, class _BanditAlgorithm_> class _TreeBandit> class Algorithm,
    int size,
    int seed,
    int depth,
    int playouts
>
void saturation_test() {
    using MoldState = MoldState<size>;
    using Model = MonteCarloModel<MoldState>;
    using Algo = Algorithm<Model, TreeBandit>;

    int tree_size = (std::pow(size*size, (depth+1)) - 1) / (size*size-1);

    MoldState state(depth);
    prng device(seed);
    Model model(device);
    MatrixNode<Algo> root;
    Algo session(device);
    session.run(playouts, state, model, root);
    const int root_count = root.count();
    std::cout << root_count << std::endl;
    assert(root_count == tree_size);
}

/*
Search in a very large tree (no revisits) should have the same number of nodes as playouts
*/
template <
    template <class Model, template <class _Model, class _BanditAlgorithm_> class _TreeBandit> class Algorithm,
    int size,
    int seed,
    int depth,
    int playouts
>
void embedding_test() {
    using MoldState = MoldState<size>;
    using Model = MonteCarloModel<MoldState>;
    using Algo = Algorithm<Model, TreeBandit>;

    int tree_size = (std::pow(size*size, (depth+1)) - 1) / (size*size-1);

    MoldState state(depth);
    prng device;
    Model model(device);
    MatrixNode<Algo> root;
    Algo session(device);
    session.run(playouts, state, model, root);
    int root_count = root.count();
    std::cout << root_count << ' ' << playouts << std::endl;
    assert(root_count == playouts);
}


int main()
{
    // Algorithm, size, seed, depth, playouts
    saturation_test<Exp3p, 2, 0, 2, 100>();
    saturation_test<Exp3p, 2, 0, 2, 100>();
    saturation_test<Exp3p, 3, 0, 2, 1000>();
    saturation_test<Exp3p, 3, 0, 3, 820 * 10>();

    embedding_test<Exp3p, 2, 0, 4, 20>();
    // embedding_test<Exp3p, 2, 0, 6, 100>();
    embedding_test<Exp3p, 3, 0, 3, 20>();
    embedding_test<Exp3p, 3, 0, 7, 10000>();

    saturation_test<MatrixUCB, 2, 0, 2, 100>();
    saturation_test<MatrixUCB, 2, 0, 2, 100>();
    saturation_test<MatrixUCB, 3, 0, 2, 1000>();
    saturation_test<MatrixUCB, 3, 0, 3, 820 * 10>();

    embedding_test<MatrixUCB, 2, 0, 4, 20>();
    // embedding_test<MatrixUCB, 2, 0, 6, 100>();
    embedding_test<MatrixUCB, 3, 0, 3, 20>();
    embedding_test<MatrixUCB, 3, 0, 7, 10000>();


}