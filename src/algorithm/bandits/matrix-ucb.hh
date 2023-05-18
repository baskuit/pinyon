#pragma once

#include <string>

#include "bandit.hh"
#include "../../tree/tree.hh"
#include "../../libsurskit/gambit.hh"
/*
MatrixUCB
*/

template <class Model, template <class _Model, class _BanditAlgorithm, class _Outcome> class _TreeBandit>
class MatrixUCB : public _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>, ChoicesOutcome<Model>>
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>, ChoicesOutcome<Model>>::Types
    {
        using MatrixStats = MatrixUCB::MatrixStats;
        using ChanceStats = MatrixUCB::ChanceStats;
    };

    struct MatrixStats : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>, ChoicesOutcome<Model>>::MatrixStats
    {
        int time = 0;
        typename Types::MatrixReal row_value_matrix;
        typename Types::MatrixReal col_value_matrix;
        typename Types::MatrixInt visit_matrix;

        typename Types::VectorReal row_strategy;
        typename Types::VectorReal col_strategy;

        typename Types::Real row_value_total = 0;
        typename Types::Real col_value_total = 0;
        int total_visits = 0;
    };

    struct ChanceStats : _TreeBandit<Model, MatrixUCB<Model, _TreeBandit>, ChoicesOutcome<Model>>::ChanceStats
    {
        // int visits = 0;
        // typename Types::Real row_value_total = 0;
        // typename Types::Real col_value_total = 0;
    };

    MatrixUCB() {}

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
        int iterations,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<MatrixUCB> *matrix_node)
    {
        matrix_node->stats.time = iterations;
    }

    void get_empirical_strategies(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        const int rows = matrix_node->row_actions.size();
        const int cols = matrix_node->col_actions.size();
        row_strategy.fill(rows, 0);
        col_strategy.fill(cols, 0);
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                const int n = matrix_node->stats.visit_matrix.get(row_idx, col_idx);
                row_strategy[row_idx] += n;
                col_strategy[col_idx] += n;
            }
        }
        math::power_norm(row_strategy, rows, 1, row_strategy);
        math::power_norm(col_strategy, cols, 1, col_strategy);

    }

    void get_empirical_values(
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Real &row_value,
        typename Types::Real &col_value)
    {
        auto stats = matrix_node->stats;
        const typename Types::Real den = 1 / (stats.total_visits + (stats.total_visits == 0));
        row_value = stats.row_value_total * den;
        col_value = stats.col_value_total * den;
    }

    void expand(
        typename Types::State &state,
        typename Types::Model model,
        MatrixNode<MatrixUCB> *matrix_node)
    {
        const int rows = state.row_actions.size();
        const int cols = state.col_actions.size();

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
                (static_cast<typename Types::Real>(matrix_node->prob));
            int time_estimate = matrix_parent->stats.time * reach_probability;
            time_estimate = time_estimate == 0 ? 1 : time_estimate;
            matrix_node->stats.time = time_estimate;
        }
    }

    void select(
        typename Types::PRNG &device,
        MatrixNode<MatrixUCB> *matrix_node,
        typename Types::Outcome &outcome)
    {
        const int rows = matrix_node->row_actions.size();
        const int cols = matrix_node->col_actions.size();
        typename Types::MatrixReal row_ucb_matrix(rows, cols);
        typename Types::MatrixReal col_ucb_matrix(rows, cols);
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
        matrix_node->stats.row_value_total += outcome.row_value;
        matrix_node->stats.col_value_total += outcome.col_value;
        ++matrix_node->stats.total_visits;
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