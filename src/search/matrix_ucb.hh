#pragma once

#include <string>

#include "libsurskit/random.hh"
#include "libsurskit/math.hh"
#include "algorithm.hh"
#include "tree/node.hh"

#include "solvers/enummixed/enummixed.h"

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
    using Solver = Gambit::Nash::EnumMixedStrategySolver<typename Types::Real>;
    struct MatrixStats : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>>::MatrixStats
    {
        int time = 0;
        typename Types::MatrixReal row_value_matrix;
        typename Types::MatrixReal col_value_matrix;
        typename Types::MatrixInt visit_matrix;

        typename Types::VectorReal row_strategy;
        typename Types::VectorReal col_strategy;
    };
    struct ChanceStats : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>>::ChanceStats
    {
        int visits = 0;
        typename Types::Real row_value_total = 0;
        typename Types::Real col_value_total = 0;
    };

    prng &device;
    Solver solver;

    MatrixUCB(prng &device) : device(device) {}

    typename Types::Real c_uct = 2;
    typename Types::Real expl_threshold = .05;
    bool require_interior = false;

    void _init_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<MatrixUCB> *matrix_node)
    {
        matrix_node->stats.time = playouts;
    }

    void _get_strategies(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy
    ) {
        row_strategy = matrix_node->stats.row_strategy;
        col_strategy = matrix_node->stats.col_strategy;
    }

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
            typename Types::Real reach_probability =
                matrix_parent->inference.row_policy[row_idx] *
                matrix_parent->inference.col_policy[col_idx] *
                ((typename Types::Real)matrix_node->transition.prob);
            int time_estimate = matrix_parent->stats.time * reach_probability;
            time_estimate = time_estimate == 0 ? 1 : time_estimate;
            matrix_node->stats.time = time_estimate;
        }
    }

    void _select(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Outcome &outcome)
    {
        typename Types::MatrixReal row_ucb_matrix;
        typename Types::MatrixReal col_ucb_matrix;
        row_ucb_matrix.rows = matrix_node->actions.rows;
        row_ucb_matrix.cols = matrix_node->actions.cols;
        col_ucb_matrix.rows = matrix_node->actions.rows;
        col_ucb_matrix.cols = matrix_node->actions.cols;

        get_ucb_matrix(
            matrix_node->stats.row_value_matrix,
            matrix_node->stats.col_value_matrix,
            matrix_node->stats.visit_matrix,
            row_ucb_matrix, // Modifies these
            col_ucb_matrix,
            matrix_node->stats.time);
        // typename Types::Real exploitability = Bandit::exploitability<typename Types::Real, MatrixUCB::state_t::_size>(
        //     bimatrix,
        //     matrix_node->stats.row_strategy,
        //     matrix_node->stats.col_strategy);
        // if (exploitability > this->expl_threshold)
        // {
        //     ++this->expl_hits;

        typename Types::VectorReal& row_strategy = matrix_node->stats.row_strategy;
        typename Types::VectorReal& col_strategy = matrix_node->stats.col_strategy;
        solve_bimatrix(
        row_ucb_matrix,
        col_ucb_matrix,
        row_strategy,
        col_strategy);
        // }
        const int row_idx = this->device.sample_pdf(row_strategy, row_ucb_matrix.rows);
        const int col_idx = this->device.sample_pdf(col_strategy, row_ucb_matrix.cols); 
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
        outcome.row_mu = row_strategy[row_idx];
        outcome.col_mu = col_strategy[col_idx];
    }

    void _update_matrix_node(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Outcome &outcome)
    {
        matrix_node->stats.row_value_matrix.data[outcome.row_idx][outcome.col_idx] += outcome.row_value;
        matrix_node->stats.col_value_matrix.data[outcome.row_idx][outcome.col_idx] += outcome.col_value;
        matrix_node->stats.visit_matrix.data[outcome.row_idx][outcome.col_idx] += 1;
    }

    void _update_chance_node(
        ChanceNode<MatrixUCB> *chance_node,
        typename Types::Outcome &outcome)
    {
    }

    void get_ucb_matrix(
        typename Types::MatrixReal &row_value_matrix,
        typename Types::MatrixReal &col_value_matrix,
        typename Types::MatrixInt &visit_matrix,
        typename Types::MatrixReal &row_ucb_matrix,
        typename Types::MatrixReal &col_ucb_matrix,
        int t)
    {
        const int rows = visit_matrix.rows;
        const int cols = visit_matrix.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                const typename Types::Real u = row_value_matrix.get(row_idx, col_idx);
                const typename Types::Real v = col_value_matrix.get(row_idx, col_idx);
                int n = visit_matrix.get(row_idx, col_idx);
                n += (n == 0);
                typename Types::Real a = u / n;
                typename Types::Real b = v / n;
                typename Types::Real const eta = this->c_uct * std::sqrt((2 * std::log(t) + std::log(2 * rows * cols)) / n);
                const typename Types::Real x = a + eta;
                const typename Types::Real y = b + eta;
                row_ucb_matrix.get(row_idx, col_idx) = x;
                col_ucb_matrix.get(row_idx, col_idx) = y;
            }
        }
    }

    void solve_bimatrix(
        typename Types::MatrixReal &row_matrix, //TODO Horrible names!!!
        typename Types::MatrixReal &col_matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        Gambit::Game game = build_nfg(row_matrix, col_matrix);
        Gambit::shared_ptr<Gambit::Nash::EnumMixedStrategySolution<typename Types::Real>> solution = solver.SolveDetailed(game); // No exceptino handling 8)
        Gambit::List<Gambit::List<Gambit::MixedStrategyProfile<typename Types::Real>>> cliques = solution->GetCliques();
        Gambit::MixedStrategyProfile<typename Types::Real> joint_strategy = cliques[1][1];
        typename Types::Real is_interior = 1.0;
        for (int i = 0; i < row_matrix.rows; ++i)
        {
            row_strategy[i] = joint_strategy[i + 1];
            is_interior *= 1 - row_strategy[i];
        }
        for (int j = row_matrix.rows; j < row_matrix.rows + row_matrix.cols; ++j)
        {
            col_strategy[j - row_matrix.rows] = joint_strategy[j + 1];
            is_interior *= 1 - col_strategy[j];
        }
        // if (is_interior == 0 && this->require_interior)
        // {
        //     ++this->gambit_hits;
        //     Bandit::SolveBimatrix<typename Types::Real, MatrixUCB::state_t::_size>(
        //         this->device,
        //         10000,
        //         bimatrix,
        //         row_strategy,
        //         col_strategy);
        // }
        delete game;
    }

    Gambit::Game build_nfg(
        typename Types::MatrixReal &row_ucb_matrix,
        typename Types::MatrixReal &col_ucb_matrix)
    {
        Gambit::Array<int> dim(2);
        dim[1] = row_ucb_matrix.rows;
        dim[2] = row_ucb_matrix.cols;
        Gambit::GameRep *nfg = NewTable(dim);
        Gambit::Game game = nfg;
        Gambit::StrategyProfileIterator iter(Gambit::StrategySupportProfile(static_cast<Gambit::GameRep *>(nfg)));
        for (int j = 0; j < row_ucb_matrix.cols; ++j)
        {
            for (int i = 0; i < row_ucb_matrix.rows; ++i)
            {
                (*iter)->GetOutcome()->SetPayoff(1, std::to_string(row_ucb_matrix.get(i, j)));
                (*iter)->GetOutcome()->SetPayoff(2, std::to_string(col_ucb_matrix.get(i, j)));
                iter++;
            }
        }
        return game;
    }
};