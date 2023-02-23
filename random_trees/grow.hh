#pragma once

#include <string>

#include "tree/node.hh"
#include "search/algorithm.hh"


#include "solvers/enummixed/enummixed.h"

using Solver = Gambit::Nash::EnumMixedStrategySolver<double>;
using Solution = Gambit::List<Gambit::List<Gambit::MixedStrategyProfile<double>>>;

template <typename Model>
class Grow : public Algorithm<Model>
{
public:
    struct MatrixStats : Algorithm<Model>::MatrixStats
    {
        bool grown = false;
        double payoff = 0;
        Linear::Bimatrix2D<double, Grow::state_t::_size> expected_value;
        std::array<double, Grow::state_t::_size> strategy0 = {0};
        std::array<double, Grow::state_t::_size> strategy1 = {0};
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats
    {
    };

    prng &device;
    Solver solver;
    bool require_interior = false;

    Grow(prng &device) : device(device)
    {
    }

    void grow(
        typename Grow::state_t &state,
        MatrixNode<Grow> *matrix_node)
    {
        if (matrix_node->stats.grown)
        {
            return;
        }
        // forward
        typename Grow::pair_actions_t legal_actions = state.get_legal_actions();
        matrix_node->stats.expected_value.rows = legal_actions.rows;
        matrix_node->stats.expected_value.cols = legal_actions.cols;
        for (int i = 0; i < legal_actions.rows; ++i)
        {
            for (int j = 0; j < legal_actions.cols; ++j)
            {
                ChanceNode<Grow> *chance_node = matrix_node->access(i, j);
                for (int c = 0; c < 1; ++c)
                {
                    typename Grow::state_t state_ = state;
                    typename Grow::transition_data_t transition_data = state_.apply_actions(i, j);
                    MatrixNode<Grow> *matrix_node_next = chance_node->access(transition_data);
                    grow(state_, matrix_node_next);
                    matrix_node->stats.expected_value.set0(i, j, matrix_node_next->stats.payoff);
                    matrix_node->stats.expected_value.set1(i, j, 1 - matrix_node_next->stats.payoff);
                }
            }
        }

        // backward
        if (legal_actions.rows * legal_actions.cols == 0)
        {
            matrix_node->stats.payoff = state.payoff0;
            matrix_node->is_terminal = true;
        }
        else
        {
            solve_bimatrix(
                matrix_node->stats.expected_value,
                matrix_node->stats.strategy0,
                matrix_node->stats.strategy1);
            for (int i = 0; i < legal_actions.rows; ++i)
            {
                for (int j = 0; j < legal_actions.cols; ++j)
                {
                    matrix_node->stats.payoff += matrix_node->stats.strategy0[i] * matrix_node->stats.strategy1[j] * matrix_node->stats.expected_value.get0(i, j);
                }
            }
        }
        matrix_node->stats.grown = true;
    }

private:

    void solve_bimatrix(
        Linear::Bimatrix2D<double, Grow::state_t::_size> &bimatrix,
        std::array<double, Grow::state_t::_size> &strategy0,
        std::array<double, Grow::state_t::_size> &strategy1)
    {
        Gambit::Game game = build_nfg(bimatrix);
        Gambit::shared_ptr<Gambit::Nash::EnumMixedStrategySolution<double>> solution = solver.SolveDetailed(game); // No exceptino handling 8)
        Solution cliques = solution->GetCliques();
        Gambit::MixedStrategyProfile<double> joint_strategy = cliques[1][1];
        double is_interior = 1.0;
        for (int i = 0; i < bimatrix.rows; ++i)
        {
            strategy0[i] = joint_strategy[i + 1];
            is_interior *= 1 - strategy0[i];
        }
        for (int j = bimatrix.rows; j < bimatrix.rows + bimatrix.cols; ++j)
        {
            strategy1[j - bimatrix.rows] = joint_strategy[j + 1];
            is_interior *= 1 - strategy1[j];
        }

        if (is_interior == 0 && this->require_interior)
        {
            Bandit::SolveBimatrix<double, Grow::state_t::_size>(
                this->device,
                10000,
                bimatrix,
                strategy0,
                strategy1);
        }
        delete game;
    }

    Gambit::Game build_nfg(
        Linear::Bimatrix2D<double, Grow::state_t::_size> bimatrix)
    {
        Gambit::Array<int> dim(2);
        dim[1] = bimatrix.rows;
        dim[2] = bimatrix.cols;
        Gambit::GameRep *nfg = NewTable(dim);
        Gambit::Game game = nfg;
        Gambit::StrategyProfileIterator iter(Gambit::StrategySupportProfile(static_cast<Gambit::GameRep *>(nfg)));
        for (int j = 0; j < bimatrix.cols; ++j)
        {
            for (int i = 0; i < bimatrix.rows; ++i)
            {
                (*iter)->GetOutcome()->SetPayoff(1, std::to_string(bimatrix.get0(i, j)));
                (*iter)->GetOutcome()->SetPayoff(2, std::to_string(bimatrix.get1(i, j)));
                iter++;
            }
        }
        return game;
    }

};