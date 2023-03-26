#include "libsurskit/random.hh"
#include "../random_trees/tree_state.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix_ucb.hh"
#include "algorithm/matrix_pucb.hh"
#include "tree/tree.hh"

#include <iostream>
#include <concepts>

template <class Algorithm1, class Algorithm2>
class DualAlgorithm
{
public:
    // static_assert(std::is_same<typename Algorithm1::Types::State, typename Algorithm2::Types::State>);
    using State = typename Algorithm1::Types::State;
    using Model1 = typename Algorithm1::Types::Model;
    using Model2 = typename Algorithm2::Types::Model;

    prng &device1;
    prng &device2;

    Model1 model1 = Model1(device1);
    Model2 model2 = Model2(device2);

    Algorithm1 session1 = Algorithm1(device1);
    Algorithm2 session2 = Algorithm2(device2);

    DualAlgorithm(prng &device1, prng &device2) : device1(device1), device2(device2) {}

    double selfplay(State &state, int playouts, double &score)
    {

        while (!state.is_terminal)
        {
            MatrixNode<Algorithm1> root1;
            MatrixNode<Algorithm2> root2;

            session1.run(playouts, device1, state, model1, root1);
            session2.run(playouts, device2, state, model2, root2);

            typename State::Types::VectorReal row_strategy;
            typename State::Types::VectorReal col_strategy;

            session1.get_strategies(&root1, row_strategy, col_strategy);
            int idx1 = device1.sample_pdf(row_strategy, state.actions.rows);
            session2.get_strategies(&root2, row_strategy, col_strategy);
            int idx2 = device2.sample_pdf(col_strategy, state.actions.cols);
            state.apply_actions(
                state.actions.row_actions[idx1],
                state.actions.col_actions[idx2]);
            state.get_actions();
        }
        return state.row_payoff;
    }

    double selfplay_flipped(State &state, int playouts, double &score)
    {

        while (!state.is_terminal)
        {
            MatrixNode<Algorithm1> root1;
            MatrixNode<Algorithm2> root2;

            session1.run(playouts, device1, state, model1, root1);
            session2.run(playouts, device2, state, model2, root2);

            typename State::Types::VectorReal row_strategy;
            typename State::Types::VectorReal col_strategy;

            session1.get_strategies(&root1, row_strategy, col_strategy);
            int idx1 = device1.sample_pdf(col_strategy, state.actions.cols);
            session2.get_strategies(&root2, row_strategy, col_strategy);
            int idx2 = device2.sample_pdf(row_strategy, state.actions.rows);
            state.apply_actions(
                state.actions.row_actions[idx2],
                state.actions.col_actions[idx1]);
            state.get_actions();
        }
        return state.col_payoff;
    }

    double selfplay_loop(State &state, int games, int playouts)
    {
        double score = 0;
        for (int game = 0; game < games; ++game)
        {
            auto state_ = state;
            score += selfplay(state_, playouts, score);
            auto state__ = state;
            score += selfplay_flipped(state__, playouts, score);
        }
        return score / (double)games / 2;
    }
};

constexpr int size = 3;

int af(prng &device, int actions) {
    const int raw = actions - device.random_int(2) + device.random_int(2);
    return std::max(std::min(raw, size), 1);
}

int dbf(prng &device, int depth_bound) {
    const int raw = depth_bound - device.random_int(2) - 1;
    return std::max(std::min(raw, depth_bound), 0);
}

template <class Algorithm1, class Algorithm2>
double vs_new_tree(prng &device)
{
    using TreeState = TreeState<size>;
    TreeState tree(device, 12, size, size, &dbf, &af);
    std::cout << "tree size: " << tree.current->stats.count << std::endl;
    tree.get_actions();

    std::cout << "Tree generated" << std::endl;

    DualAlgorithm<Algorithm1, Algorithm2> eval(device, device);
    // eval.session1.c_uct = 1.718;
    double result = eval.selfplay_loop(tree, 30, 200);
    return result;
}

int main()
{
    using TreeState = TreeState<3>;
    using Model = SolvedMonteCarloModel<TreeState>;
    using Algorithm1 = MatrixPUCB<Model, TreeBandit>;
    using Algorithm2 = MatrixUCB<Model, TreeBandit>;

    prng device(0);

    double v = 0;
    int n = 0;

    for (int i = 0; i < 50; ++i)
    {
        double u = vs_new_tree<Algorithm1, Algorithm2>(device);
        v += u;
        ++n;
        std::cout << v / n << std::endl;
    }

    return 0;
}