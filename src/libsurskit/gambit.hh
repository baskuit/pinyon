#pragma once

#include "math.hh"
#include "solvers/enummixed/enummixed.h"
#include "state/test_states.hh"

namespace LibGambit {

template <class TypeList>
void solve_bimatrix(
    typename TypeList::MatrixReal &row_payoff_matrix,
    typename TypeList::MatrixReal &col_payoff_matrix,
    typename TypeList::VectorReal &row_strategy,
    typename TypeList::VectorReal &col_strategy)
{
    Gambit::Nash::EnumMixedStrategySolver<typename TypeList::Real> solver;
    Gambit::Game game = build_nfg<TypeList>(row_payoff_matrix, col_payoff_matrix);
    Gambit::shared_ptr<Gambit::Nash::EnumMixedStrategySolution<typename TypeList::Real>> solution = solver.SolveDetailed(game); // No exceptino handling 8)
    try
    {
        Gambit::List<Gambit::List<Gambit::MixedStrategyProfile<typename TypeList::Real>>> cliques = solution->GetCliques();
        Gambit::MixedStrategyProfile<typename TypeList::Real> joint_strategy = cliques[1][1];
        typename TypeList::Real is_interior = 1.0;
        for (int i = 0; i < row_payoff_matrix.rows; ++i)
        {
            row_strategy[i] = joint_strategy[i + 1];
            is_interior *= 1 - row_strategy[i];
        }
        for (int j = row_payoff_matrix.rows; j < row_payoff_matrix.rows + row_payoff_matrix.cols; ++j)
        {
            col_strategy[j - row_payoff_matrix.rows] = joint_strategy[j + 1];
            is_interior *= 1 - col_strategy[j];
        }
    }
    catch (Gambit::IndexException)
    {
        BimatrixGame<TypeList> matrix_game(row_payoff_matrix, col_payoff_matrix);
        matrix_game.solve(row_strategy, col_strategy);
    }
    delete game;
}

template <class TypeList>
Gambit::Game build_nfg(
    typename TypeList::MatrixReal &row_payoff_matrix,
    typename TypeList::MatrixReal &col_payoff_matrix)
{
    Gambit::Array<int> dim(2);
    dim[1] = row_payoff_matrix.rows;
    dim[2] = row_payoff_matrix.cols;
    Gambit::GameRep *nfg = NewTable(dim);
    Gambit::Game game = nfg;
    Gambit::StrategyProfileIterator iter(Gambit::StrategySupportProfile(static_cast<Gambit::GameRep *>(nfg)));
    for (int j = 0; j < row_payoff_matrix.cols; ++j)
    {
        for (int i = 0; i < row_payoff_matrix.rows; ++i)
        {
            (*iter)->GetOutcome()->SetPayoff(1, std::to_string(row_payoff_matrix.get(i, j)));
            (*iter)->GetOutcome()->SetPayoff(2, std::to_string(col_payoff_matrix.get(i, j)));
            iter++;
        }
    }
    return game;
}
}; // End namespace Gambit