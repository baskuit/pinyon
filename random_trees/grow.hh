#pragma once

#include <string>

#include "tree/node.hh"
#include "search/algorithm.hh"
// #include "model/model.hh"

#include "solvers/enummixed/enummixed.h"

using Solver = Gambit::Nash::EnumMixedStrategySolver<double>;
using Solution = Gambit::List<Gambit::List<Gambit::MixedStrategyProfile<double>>>;

template <typename Model>
class Grow : public AbstractAlgorithm<Model>
{
    static_assert(std::derived_from<typename Model::Types::State, SeedState<Model::Types::size>>);

public:
    struct MatrixStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = Grow::MatrixStats;
    };
    struct MatrixStats : AbstractAlgorithm<Model>::MatrixStats
    {
        bool grown = false;
        typename Types::Real payoff = 0;
        typename Types::MatrixReal expected_value;
        typename Types::VectorReal row_strategy = {0};
        typename Types::VectorReal col_strategy = {0};
    };

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
        if (matrix_node->stats.grown)
        {
            return;
        }
        // forward
        state.get_actions();
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
                    MatrixNode<Grow> *matrix_node_next = chance_node->access(state.transition);
                    grow(state_, matrix_node_next);
                    matrix_node->stats.expected_value.data[i][j] = matrix_node_next->stats.payoff;
                }
            }
        }

        // backward
        if (rows * cols == 0)
        {
            matrix_node->stats.payoff = state.row_payoff;
            matrix_node->is_terminal = true;
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
        matrix_node->stats.grown = true;
    }

private:
    void solve_matrix(
        typename Types::MatrixReal &matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        // Gambit::Game game = build_nfg(matrix);
        // Gambit::shared_ptr<Gambit::Nash::EnumMixedStrategySolution<double>> solution = solver.SolveDetailed(game); // No exceptino handling 8)
        // Solution cliques = solution->GetCliques();
        // Gambit::MixedStrategyProfile<double> joint_strategy = cliques[1][1];
        // double is_interior = 1.0;
        // for (int i = 0; i < matrix.rows; ++i)
        // {
        //     row_strategy[i] = joint_strategy[i + 1];
        //     is_interior *= 1 - row_strategy[i];
        // }
        // for (int j = matrix.rows; j < matrix.rows + matrix.cols; ++j)
        // {
        //     col_strategy[j - matrix.rows] = joint_strategy[j + 1];
        //     is_interior *= 1 - col_strategy[j];
        // }

        // // if (is_interior == 0 && this->require_interior)
        // // {
        // //     Bandit::SolveBimatrix<double, Grow::state_t::_size>(
        // //         this->device,
        // //         10000,
        // //         bimatrix,
        // //         row_strategy,
        // //         col_strategy);
        // // }
        // delete game;
    }

    //     Gambit::Game build_nfg(
    //         Linear::Bimatrix2D<double, Grow::state_t::_size> bimatrix)
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
};