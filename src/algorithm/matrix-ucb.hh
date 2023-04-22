#pragma once

#include <string>

#include "libsurskit/random.hh"
#include "libsurskit/math.hh"
#include "libsurskit/gambit.hh"
#include "state/test-states.hh"
#include "algorithm.hh"
#include "tree/tree.hh"

/*
MatrixUCB
*/

template <class Model, template <class _Model, class _BanditAlgorithm> class _TreeBandit>
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
        typename Types::MatrixInt visit_matrix;

        typename Types::VectorReal row_strategy;
        typename Types::VectorReal col_strategy;
    };

    struct ChanceStats : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>>::ChanceStats
    {
        // int visits = 0;
        // typename Types::Real row_value_total = 0;
        // typename Types::Real col_value_total = 0;
    };

    MatrixUCB () {}

    MatrixUCB(typename Types::Real c_uct, typename Types::Real expl_threshold) : c_uct(c_uct), expl_threshold(expl_threshold) {}

    friend std::ostream &operator<<(std::ostream &os, const MatrixUCB &session)
    {
        os << "MatrixUCB; c_uct: " << session.c_uct << ", expl_threshold: " << session.expl_threshold;
        return os;
    }

    const typename Types::Real c_uct = 1;
    const typename Types::Real expl_threshold = .005;
    // bool require_interior = false;

    void initialize_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<MatrixUCB> *matrix_node)
    {
        matrix_node->stats.time = playouts;
    }

    void get_strategies(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        typename Types::MatrixReal row_ev_matrix(matrix_node->actions.rows, matrix_node->actions.cols);
        typename Types::MatrixReal col_ev_matrix(matrix_node->actions.rows, matrix_node->actions.cols);
        get_ev_matrix(
            matrix_node,
            row_ev_matrix,
            col_ev_matrix);
        LibGambit::solve_bimatrix<Types>(
            row_ev_matrix,
            col_ev_matrix,
            row_strategy,
            col_strategy);
    }

    void expand(
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

        matrix_node->stats.row_value_matrix.fill(rows, cols, 0);
        matrix_node->stats.col_value_matrix.fill(rows, cols, 0);
        matrix_node->stats.visit_matrix.fill(rows, cols, 0);

        // Uniform initialization of stats.strategies
        matrix_node->stats.row_strategy.fill(rows, 1 / static_cast<typename Types::Real>(rows));
        matrix_node->stats.col_strategy.fill(cols, 1 / static_cast<typename Types::Real>(cols));

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
                (static_cast<typename Types::Real>(matrix_node->transition.prob));
            int time_estimate = matrix_parent->stats.time * reach_probability;
            time_estimate = time_estimate == 0 ? 1 : time_estimate;
            matrix_node->stats.time = time_estimate;
        }
    }

    void select(
        prng &device,
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Outcome &outcome)
    {
        typename Types::MatrixReal row_ucb_matrix(matrix_node->actions.rows, matrix_node->actions.cols);
        typename Types::MatrixReal col_ucb_matrix(matrix_node->actions.rows, matrix_node->actions.cols);
        get_ucb_matrix(
            matrix_node,
            row_ucb_matrix,
            col_ucb_matrix);
        typename Types::VectorReal &row_strategy = matrix_node->stats.row_strategy;
        typename Types::VectorReal &col_strategy = matrix_node->stats.col_strategy;
        typename Types::Real expl = Linear::exploitability<Types>(row_ucb_matrix, col_ucb_matrix, row_strategy, col_strategy);
        if (expl > expl_threshold)
        {
            LibGambit::solve_bimatrix<Types>(
                row_ucb_matrix,
                col_ucb_matrix,
                row_strategy,
                col_strategy);
        }
        const int row_idx = device.sample_pdf(row_strategy, row_ucb_matrix.rows);
        const int col_idx = device.sample_pdf(col_strategy, row_ucb_matrix.cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
    }

    void update_matrix_node(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Outcome &outcome)
    {
        matrix_node->stats.row_value_matrix.get(outcome.row_idx, outcome.col_idx) += outcome.row_value;
        matrix_node->stats.col_value_matrix.get(outcome.row_idx, outcome.col_idx) += outcome.col_value;
        matrix_node->stats.visit_matrix.get(outcome.row_idx, outcome.col_idx) += 1;
    }

    void update_chance_node(
        ChanceNode<MatrixUCB> *chance_node,
        typename Types::Outcome &outcome)
    {
        // chance_node->stats.row_value_total += outcome.row_value;
        // chance_node->stats.col_value_total += outcome.col_value;
        // ++chance_node->stats.visits;
    }

// private:
    void get_ucb_matrix(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::MatrixReal &row_ucb_matrix,
        typename Types::MatrixReal &col_ucb_matrix)
    {
        typename Types::MatrixReal &row_value_matrix = matrix_node->stats.row_value_matrix;
        typename Types::MatrixReal &col_value_matrix = matrix_node->stats.col_value_matrix;
        typename Types::MatrixInt &visit_matrix = matrix_node->stats.visit_matrix;
        const int time = matrix_node->stats.time;
        const int rows = visit_matrix.rows;
        const int cols = visit_matrix.cols;
        const typename Types::Real num = 2 * std::log(time) + std::log(2 * rows * cols);
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
                typename Types::Real const eta = this->c_uct * std::sqrt(num / n);
                const typename Types::Real x = a + eta;
                const typename Types::Real y = b + eta;
                row_ucb_matrix.get(row_idx, col_idx) = x;
                col_ucb_matrix.get(row_idx, col_idx) = y;
            }
        }
    }

    void get_ev_matrix(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::MatrixReal &row_ev_matrix,
        typename Types::MatrixReal &col_ev_matrix)
    {
        typename Types::MatrixReal &row_value_matrix = matrix_node->stats.row_value_matrix;
        typename Types::MatrixReal &col_value_matrix = matrix_node->stats.col_value_matrix;
        typename Types::MatrixInt &visit_matrix = matrix_node->stats.visit_matrix;
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
                row_ev_matrix.get(row_idx, col_idx) = a;
                col_ev_matrix.get(row_idx, col_idx) = b;
            }
        }
    }
};