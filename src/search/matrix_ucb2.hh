#pragma once

#include <string>

#include "libsurskit/random.hh"
#include "libsurskit/math.hh"
#include "algorithm.hh"
#include "tree/node.hh"

// #include "solvers/enummixed/enummixed.h"

// using Solver = Gambit::Nash::EnumMixedStrategySolver<double>;

/*
MatrixUCB
*/

// TODO MAKE THREAD SAFE (how?)

template <class Model, template <class _Model, class _BanditAlgorithm_> class _TreeBandit>
class MatrixUCB : public _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>>
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>>::Types
    {
        using MatrixStats = MatrixUCB::MatrixStats;
        using ChanceStats = MatrixUCB::ChanceStats;
    };
    struct MatrixStats : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>>::MatrixStats
    {
        int time = 0;
        typename Types::MatrixReal row_value_matrix;
        typename Types::MatrixReal col_value_matrix;
        typename Types::MatrixReal visit_matrix;

        typename Types::VectorReal row_strategy;
        typename Types::VectorReal col_strategy;
    };

    struct ChanceStats : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>>::ChanceStats
    {
        int visits = 0;
        double row_value_total = 0;
        double col_value_total = 0;
    };

    prng &device;
    // Solver solver;

    MatrixUCB(prng &device) : device(device) {}

    double c_uct = 2;
    double expl_threshold = .05;
    bool require_interior = false;

    void _init_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<MatrixUCB> *matrix_node)
    {
        matrix_node->stats.time = playouts;
    }

    // Old imp

    //     void search(
    //         int playouts,
    //         typename MatrixUCB::state_t &state,
    //         typename MatrixUCB::model_t &model,
    //         MatrixNode<MatrixUCB> &root)
    //     {
    //         root.stats.t = playouts;
    //         for (int playout = 0; playout < playouts; ++playout)
    //         {
    //             auto state_ = state;
    //             runPlayout(state_, model, &root);
    //         }
    //         std::cout << "begin search output" << std::endl;
    //         Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix =
    //             get_matrix(
    //                 root.stats.cumulative_payoffs,
    //                 root.stats.visits);
    //         std::cout << "expected value matrix:" << std::endl;
    //         bimatrix.print();
    //         std::cout << "visit matrix" << std::endl;
    //         root.stats.visits.print();
    //         std::cout << "strategies" << std::endl;
    //         for (int i = 0; i < bimatrix.rows; ++i) {
    //             std::cout << root.stats.row_strategy[i] << ' ';
    //         }
    //         std::cout << std::endl;
    //         for (int j = 0; j < bimatrix.cols; ++j) {
    //             std::cout << root.stats.col_strategy[j] << ' ';
    //         }
    //         std::cout << std::endl;
    //     }

    // private:
    //     MatrixNode<MatrixUCB> *runPlayout(
    //         typename MatrixUCB::state_t &state,
    //         typename MatrixUCB::model_t &model,
    //         MatrixNode<MatrixUCB> *matrix_node)
    //     {

    //         if (matrix_node->is_terminal == true)
    //         {
    //             return matrix_node;
    //         }
    //         else
    //         {

    //             if (matrix_node->is_expanded == true)
    //             {
    //                 Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix(matrix_node->legal_actions.rows, matrix_node->legal_actions.cols);

    //                 get_ucb_matrix(
    //                     bimatrix,
    //                     matrix_node->stats.cumulative_payoffs,
    //                     matrix_node->stats.visits,
    //                     matrix_node->stats.t);
    //                 double exploitability = Bandit::exploitability<double, MatrixUCB::state_t::_size>(
    //                     bimatrix,
    //                     matrix_node->stats.row_strategy,
    //                     matrix_node->stats.col_strategy);
    //                 if (exploitability > this->expl_threshold)
    //                 {
    //                     ++this->expl_hits;
    //                     solve_bimatrix(bimatrix, matrix_node->stats.row_strategy, matrix_node->stats.col_strategy);
    //                 }

    //                 int row_idx = device.sample_pdf<double, MatrixUCB::state_t::_size>(matrix_node->stats.row_strategy, matrix_node->legal_actions.rows);
    //                 int col_idx = device.sample_pdf<double, MatrixUCB::state_t::_size>(matrix_node->stats.col_strategy, matrix_node->legal_actions.cols);
    //                 typename MatrixUCB::action_t action0 = matrix_node->legal_actions.actions0[row_idx];
    //                 typename MatrixUCB::action_t action1 = matrix_node->legal_actions.actions1[col_idx];
    //                 typename MatrixUCB::transition_data_t transition_data = state.apply_actions(action0, action1);

    //                 ChanceNode<MatrixUCB> *chance_node = matrix_node->access(row_idx, col_idx);
    //                 MatrixNode<MatrixUCB> *matrix_node_next = chance_node->access(transition_data);
    //                 MatrixNode<MatrixUCB> *matrix_node_leaf = runPlayout(state, model, matrix_node_next);

    //                 double u0 = matrix_node_leaf->inference.value0;
    //                 double u1 = matrix_node_leaf->inference.value1;
    //                 update(matrix_node, u0, u1, row_idx, col_idx);
    //                 return matrix_node_leaf;
    //             }
    //             else
    //             {
    //                 expand(state, model, matrix_node);
    //                 return matrix_node;
    //             }
    //         }
    //     }

    void _expand(
        typename Types::State &state,
        typename Types::Model model,
        MatrixNode<MatrixUCB> *matrix_node)
    {
        matrix_node->is_expanded = true;
        state.get_actions();
        matrix_node->is_terminal = state.is_terminal;
        matrix_node->actions = state.actions;
        const int rows = state.actions.rows;
        const int cols = state.actions.cols;

        if (matrix_node->is_terminal)
        {
            matrix_node->inference.row_value = state.row_payoff;
            matrix_node->inference.col_value = state.col_payoff;
        }
        else
        {
            model.get_inference(state, matrix_node->inference);
        }

        matrix_node->stats.row_value_matrix.rows = rows;
        matrix_node->stats.row_value_matrix.cols = cols;
        matrix_node->stats.col_value_matrix.rows = rows;
        matrix_node->stats.col_value_matrix.cols = cols;
        matrix_node->stats.visit_matrix.rows = rows;
        matrix_node->stats.visit_matrix.cols = cols;

        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                matrix_node->stats.row_value_matrix.data[row_idx][col_idx] = 0;
                matrix_node->stats.col_value_matrix.data[row_idx][col_idx] = 0;
                matrix_node->stats.visit_matrix.data[row_idx][col_idx] = 0;
            }
        }

        // Uniform initialization of stats.strategies
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            matrix_node->stats.row_strategy[row_idx] = 1 / (float)matrix_node->actions.rows;
        }
        for (int col_idx = 0; col_idx < cols; ++col_idx)
        {
            matrix_node->stats.col_strategy[col_idx] = 1 / (float)matrix_node->actions.cols;
        }

        // Calculate node's time parameter using parent's.
        ChanceNode<MatrixUCB> *chance_parent = matrix_node->parent;
        if (chance_parent != nullptr)
        {
            MatrixNode<MatrixUCB> *matrix_parent = chance_parent->parent;
            int row_idx = chance_parent->row_idx;
            int col_idx = chance_parent->col_idx;
            double reach_probability =
                matrix_parent->inference.row_policy[row_idx] *
                matrix_parent->inference.col_policy[col_idx] *
                ((double)matrix_node->transition.prob);
            int time_estimate = matrix_parent->stats.time * reach_probability;
            time_estimate = time_estimate == 0 ? 1 : time_estimate;
            matrix_node->stats.time = time_estimate;
        }
    }

    void _select(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Outcome &outcome)
    {
        typename Types::MatrixReal row_ucb(matrix_node->actions->rows, matrix_node->actions->cols);
        typename Types::MatrixReal col_ucb(matrix_node->actions->rows, matrix_node->actions->cols);
        // bimatrix(matrix_node->legal_actions.rows, matrix_node->legal_actions.cols);

        // get_ucb_matrix(
        //     bimatrix,
        //     matrix_node->stats.cumulative_payoffs,
        //     matrix_node->stats.visits,
        //     matrix_node->stats.t);
        // double exploitability = Bandit::exploitability<double, MatrixUCB::state_t::_size>(
        //     bimatrix,
        //     matrix_node->stats.row_strategy,
        //     matrix_node->stats.col_strategy);
        // if (exploitability > this->expl_threshold)
        // {
        //     ++this->expl_hits;
        //     solve_bimatrix(bimatrix, matrix_node->stats.row_strategy, matrix_node->stats.col_strategy);
        // }

        // int row_idx = device.sample_pdf<double, MatrixUCB::state_t::_size>(matrix_node->stats.row_strategy, matrix_node->legal_actions.rows);
        // int col_idx = device.sample_pdf<double, MatrixUCB::state_t::_size>(matrix_node->stats.col_strategy, matrix_node->legal_actions.cols);
    }

    void _update_matrix_node(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Outcome &outcome)
    {
        matrix_node->stats.row_value_matrix.data[outcome.row_idx][outcome.col_idx] += outcome.row_value;
        matrix_node->stats.col_value_matrix.data[outcome.row_idx][outcome.col_idx] += outcome.col_value;
        matrix_node->stats.visit.data[outcome.row_idx][outcome.col_idx] += 1;
    }

    void _update_chance_node(
        ChanceNode<MatrixUCB> *chance_node,
        typename Types::Outcome &outcome)
    {
    }

    //     void solve_bimatrix(
    //         Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> &bimatrix,
    //         std::array<double, MatrixUCB::state_t::_size> &row_strategy,
    //         std::array<double, MatrixUCB::state_t::_size> &col_strategy)
    //     {
    //         Gambit::Game game = build_nfg(bimatrix);
    //         Gambit::shared_ptr<Gambit::Nash::EnumMixedStrategySolution<double>> solution = solver.SolveDetailed(game); // No exceptino handling 8)
    //         Gambit::List<Gambit::List<Gambit::MixedStrategyProfile<double>>> cliques = solution->GetCliques();
    //         Gambit::MixedStrategyProfile<double> joint_strategy = cliques[1][1];
    //         double is_interior = 1.0;
    //         for (int i = 0; i < bimatrix.rows; ++i)
    //         {
    //             row_strategy[i] = joint_strategy[i + 1];
    //             is_interior *= 1 - row_strategy[i];
    //         }
    //         for (int j = bimatrix.rows; j < bimatrix.rows + bimatrix.cols; ++j)
    //         {
    //             col_strategy[j - bimatrix.rows] = joint_strategy[j + 1];
    //             is_interior *= 1 - col_strategy[j];
    //         }

    //         if (is_interior == 0 && this->require_interior)
    //         {
    //             ++this->gambit_hits;
    //             Bandit::SolveBimatrix<double, MatrixUCB::state_t::_size>(
    //                 this->device,
    //                 10000,
    //                 bimatrix,
    //                 row_strategy,
    //                 col_strategy);
    //         }
    //         delete game;
    //     }

    //     Gambit::Game build_nfg(
    //         Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix)
    //     {
    //         Gambit::Array<int> dim(2);
    //         dim[1] = bimatrix.rows;
    //         dim[2] = bimatrix.cols;
    //         Gambit::GameRep *nfg = NewTable(dim);
    //         Gambit::Game game = nfg;
    //         Gambit::StrategyProfileIterator iter(Gambit::StrategySupportProfile(static_cast<Gambit::GameRep *>(nfg)));
    //         for (int j = 0; j < bimatrix.cols; ++j)
    //         {
    //             for (int i = 0; i < bimatrix.rows; ++i)
    //             {
    //                 (*iter)->GetOutcome()->SetPayoff(1, std::to_string(bimatrix.get0(i, j)));
    //                 (*iter)->GetOutcome()->SetPayoff(2, std::to_string(bimatrix.get1(i, j)));
    //                 iter++;
    //             }
    //         }
    //         return game;
    //     }

    //     // Bimatrix of UCB Scores (EV + Exploration)
    //     void get_ucb_matrix(
    //         Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> &bimatrix,
    //         Linear::Bimatrix<double, MatrixUCB::state_t::_size> &cumulative_payoffs,
    //         Linear::Matrix<int, MatrixUCB::state_t::_size> &visits,
    //         int t)
    //     {
    //         const int rows = bimatrix.rows;
    //         const int cols = bimatrix.cols;
    //         for (int row_idx = 0; row_idx < rows; ++row_idx)
    //         {
    //             for (int col_idx = 0; col_idx < cols; ++col_idx)
    //             {
    //                 double u = cumulative_payoffs.get0(row_idx, col_idx);
    //                 double v = cumulative_payoffs.get1(row_idx, col_idx);
    //                 int n = visits.get(row_idx, col_idx);
    //                 if (n == 0)
    //                 {
    //                     n = 1;
    //                 }
    //                 double a = u / n;
    //                 double b = v / n;
    //                 double const eta = this->c_uct * std::sqrt((2 * std::log(t) + std::log(2 * rows * cols)) / n);
    //                 const double x = a + eta;
    //                 const double y = b + eta;
    //                 bimatrix.set0(row_idx, col_idx, x);
    //                 bimatrix.set1(row_idx, col_idx, y);
    //             }
    //         }
    // 
    //     }
};