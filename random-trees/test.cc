#include "seed-state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/exp3.hh"
#include "algorithm/matrix-ucb.hh"

#include "grow.hh"

#include <iostream>

template <class Algorithm>
void print_matrix (MatrixNode<Algorithm> *matrix_node) {
    for (int row_idx = 0; row_idx < matrix_node->actions.rows; ++row_idx) {
        for (int col_idx = 0; col_idx < matrix_node->actions.cols; ++col_idx) {
            auto chance_node = matrix_node->access(row_idx, col_idx);
            auto next = chance_node->child;
            double value = 0;
            while (next != nullptr) {
                value += next->inference.row_value;
                next = next->next;
            }
            std::cout << value << ' ';
        }
        std::cout << std::endl;
    }
}

int main()
{

    const int MaxActions = 4;
    const int MaxTransitions = 2;
    const int depth_bound = 1;
    const double chance_threshold = 1 / (double) 3; 

    using SeedState = SeedState<MaxActions, MaxTransitions>;
    using Model = MonteCarloModel<SeedState>;
    using MatrixUCB = MatrixUCB<Model, TreeBandit>;
    using Exp3p = Exp3p<Model, TreeBandit>;
    using Algorithm = Exp3p;

    SeedState::Types::VectorReal row_strategy(MaxActions), col_strategy(MaxActions);
    SeedState::Types::MatrixReal row_value_matrix(MaxActions, MaxActions), col_value_matrix(MaxActions, MaxActions);

    prng init_device;
    int seed = 0;//init_device.uniform_64();
    std::cout << "seed: " << seed << std::endl;
    prng device(seed);
    int games = 20;

    for (int iterations = 100; iterations <= 1000000; iterations *= 10) {
        for (int actions = 2; actions <= MaxActions; ++actions) {
            for (int depth_bound = 1; depth_bound < 4; ++depth_bound) {


                double total_exp3p_expl = 0;
                double total_matrix_ucb_expl = 0;

                for (int game = 0; game < games; ++game) {

                    SeedState state(device, depth_bound, MaxActions, MaxActions, chance_threshold);
                    Model model(device);

                    Grow<Model> grow;
                    MatrixNode<Grow<Model>> grow_root;
                    grow.grow(state, model, &grow_root);

                    auto row_solution = grow_root.stats.row_solution;
                    auto col_solution = grow_root.stats.col_solution;


                    SeedState::Types::MatrixReal row_payoff_matrix = grow_root.stats.nash_payoff_matrix;
                    SeedState::Types::MatrixReal col_payoff_matrix = row_payoff_matrix * -1 + 1;


                    Exp3p exp3p_session;
                    MatrixNode<Exp3p> exp3p_root;
                    exp3p_session.run(iterations, device, state, model, exp3p_root);
                    exp3p_session.get_ev_matrix(&exp3p_root, row_value_matrix, col_value_matrix);
                    exp3p_session.get_strategies(&exp3p_root, row_strategy, col_strategy);


                    double exp3p_expl = Linear::exploitability<SeedState::Types::TypeList>(
                        row_payoff_matrix, 
                        col_payoff_matrix, 
                        row_strategy, 
                        col_strategy);



                    MatrixUCB matrix_ucb_session;
                    MatrixNode<MatrixUCB> matrix_ucb_root;
                    matrix_ucb_session.run(iterations, device, state, model, matrix_ucb_root);
                    matrix_ucb_session.get_ev_matrix(&matrix_ucb_root, row_value_matrix, col_value_matrix);
                    matrix_ucb_session.get_strategies(&matrix_ucb_root, row_strategy, col_strategy);

                    double matrix_ucb_expl = Linear::exploitability<SeedState::Types::TypeList>(
                        row_payoff_matrix, 
                        col_payoff_matrix, 
                        row_strategy, 
                        col_strategy);

                        total_exp3p_expl += exp3p_expl;
                        total_matrix_ucb_expl += matrix_ucb_expl;
                }

                std::cout << "iter: " << iterations << " actions: " << actions << " depth_bound: " << depth_bound << std::endl;
                std::cout << "mean expl exp3p: " << total_exp3p_expl / games << std::endl;
                std::cout << "mean expl matrix_ucb: " << total_matrix_ucb_expl / games << std::endl;


            }
        }
    }
    return 0;
}