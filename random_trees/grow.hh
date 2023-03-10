#pragma once

#include <string>

#include "tree/node.hh"
#include "search/algorithm.hh"

#include "solvers/enummixed/enummixed.h"

template <typename Model>
class Grow : public AbstractAlgorithm<Model>
{
    static_assert(std::derived_from<typename Model::Types::State, SeedState<Model::Types::size>>);
    // Model::State is based on SeedState

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
        typename Types::VectorReal row_strategy = {0};
        typename Types::VectorReal col_strategy = {0};
        int count = 1;
    };

    using Solver = Gambit::Nash::EnumMixedStrategySolver<double>;
    using Solution = Gambit::List<Gambit::List<Gambit::MixedStrategyProfile<double>>>;

    prng &device;
    Solver solver;
    bool require_interior = false;

    Grow(prng &device) : device(device)
    {
    }

    void grow(
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
        matrix_node->stats.expected_value.rows = rows;
        matrix_node->stats.expected_value.cols = cols;
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
            ChanceNode<Grow> *chance_node = matrix_node->access(i, j);
                for (int c = 0; c < 1; ++c)
                {
                    typename Types::State state_ = state;
                    state_.apply_actions(i, j);
                    MatrixNode<Grow> *matrix_node_next = chance_node->access(state_.transition);
                    grow(state_, matrix_node_next);
                    matrix_node->stats.expected_value.data[i][j] = matrix_node_next->stats.payoff;
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
            solve_matrix(
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
                        matrix_node->stats.expected_value.data[i][j];
                }
            }
        }
    }

private:
    void solve_matrix(
        typename Types::MatrixReal &matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        Gambit::Game game = build_nfg(matrix);
        Gambit::shared_ptr<Gambit::Nash::EnumMixedStrategySolution<double>> solution = this->solver.SolveDetailed(game);
        Solution cliques = solution->GetCliques();
        Gambit::MixedStrategyProfile<double> joint_strategy = cliques[1][1];
        double is_interior = 1.0;
        for (int i = 0; i < matrix.rows; ++i)
        {
            row_strategy[i] = joint_strategy[i + 1];
            is_interior *= 1 - row_strategy[i];
        }
        for (int j = matrix.rows; j < matrix.rows + matrix.cols; ++j)
        {
            col_strategy[j - matrix.rows] = joint_strategy[j + 1];
            is_interior *= 1 - col_strategy[j];
        }

        // if (is_interior == 0 && this->require_interior)
        // {
        //     Bandit::SolveBimatrix<double, Grow::state_t::_size>(
        //         this->device,
        //         10000,
        //         matrix,
        //         row_strategy,
        //         col_strategy);
        // }
        delete game;
    }

        Gambit::Game build_nfg(
            typename Types::MatrixReal &matrix)
        {
            Gambit::Array<int> dim(2);
            dim[1] = matrix.rows;
            dim[2] = matrix.cols;
            Gambit::GameRep *nfg = NewTable(dim);
            Gambit::Game game = nfg;
            Gambit::StrategyProfileIterator iter(Gambit::StrategySupportProfile(static_cast<Gambit::GameRep *>(nfg)));
            for (int j = 0; j < matrix.cols; ++j)
            {
                for (int i = 0; i < matrix.rows; ++i)
                {   
                    const typename Types::Real x = matrix.data[i][j];
                    (*iter)->GetOutcome()->SetPayoff(1, std::to_string(x));
                    (*iter)->GetOutcome()->SetPayoff(2, std::to_string(1 - x));
                    iter++;
                }
            }
            return game;
        }
};