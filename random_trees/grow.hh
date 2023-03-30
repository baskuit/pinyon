#pragma once

#include <string>

#include "libsurskit/gambit.hh"
#include "tree/tree.hh"
#include "algorithm/algorithm.hh"

template <typename Model>
class Grow : public AbstractAlgorithm<Model>
{
    static_assert(std::derived_from<typename Model::Types::State, SeedState<Model::Types::size>>,
                  "Grow is only intended to run on a SeedState");

public:
    struct MatrixStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = Grow::MatrixStats;
    };
    struct MatrixStats : AbstractAlgorithm<Model>::MatrixStats
    {
        typename Types::Real payoff = 0;
        typename Types::MatrixReal expected_value;
        typename Types::VectorReal row_strategy;
        typename Types::VectorReal col_strategy;
        int count = 1;
    };

    Grow() {}

    void grow(
        prng &device,
        typename Types::State &state,
        MatrixNode<Grow> *matrix_node)
    {
        if (matrix_node->is_expanded)
        {
            return;
        }
        // forward
        state.get_actions();
        matrix_node->actions = state.actions;
        matrix_node->is_expanded = true;
        matrix_node->is_terminal = state.is_terminal;

        const int rows = state.actions.rows;
        const int cols = state.actions.cols;
        // matrix_node->stats.row_strategy.fill(rows);
        // matrix_node->stats.col_strategy.fill(cols);
        matrix_node->stats.expected_value.rows = rows;
        matrix_node->stats.expected_value.cols = cols;
        // matrix_node->stats.expected_value.fill(rows, cols);
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                ChanceNode<Grow> *chance_node = matrix_node->access(i, j);
                for (int c = 0; c < 1; ++c)
                {
                    typename Types::State state_copy = state;
                    state_copy.apply_actions(i, j);
                    MatrixNode<Grow> *matrix_node_next = chance_node->access(state_copy.transition);
                    grow(device, state_copy, matrix_node_next);
                    matrix_node->stats.expected_value.get(i, j) = matrix_node_next->stats.payoff;
                    matrix_node->stats.count += matrix_node_next->stats.count;
                }
            }
        }

        // backward
        if (rows * cols == 0)
        {
            matrix_node->stats.payoff = state.row_payoff;
        }
        else
        {
            LibGambit::solve_matrix<Types>(
                matrix_node->stats.expected_value,
                matrix_node->stats.row_strategy,
                matrix_node->stats.col_strategy);
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    matrix_node->stats.payoff +=
                        matrix_node->stats.row_strategy[i] *
                        matrix_node->stats.col_strategy[j] *
                        matrix_node->stats.expected_value.get(i, j);
                }
            }
        }
    }
};