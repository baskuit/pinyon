#pragma once

#include "model/model.hh"
#include "seed_state.hh"
#include "grow.hh"

#include "shared_ptr.h"

template <int size>
class TreeState : public SolvedStateArray<size, int, int, double>
{
public:

    using SeedStateNode = MatrixNode<Grow<MonteCarloModel<SeedState<size>>>>;

    struct Types : SolvedStateArray<size, int, int, double>::Types
    {
    };

    SeedState<size> seed;
    std::shared_ptr<SeedStateNode> root;
    SeedStateNode *current;
    // Trying out make shared to keep copy assignment but also have automatic memory management

    TreeState(prng &device, int depth_bound, int rows, int cols) : seed(SeedState<size>(device, depth_bound, rows, cols))
    {
        this->root = std::make_shared<SeedStateNode>();
        this->current = &*root;
        Grow<MonteCarloModel<SeedState<size>>> session(device);
        session.grow(seed, current);
        this->row_strategy = current->stats.row_strategy;
        this->col_strategy = current->stats.col_strategy;

        update_solved_state_payoffs(current);
    }

    void get_actions()
    {
        this->actions.rows = this->current->stats.expected_value.rows;
        this->actions.cols = this->current->stats.expected_value.cols;
        for (int i = 0; i < this->actions.rows; ++i)
        {
            this->actions.row_actions[i] = i;
        };
        for (int j = 0; j < this->actions.cols; ++j)
        {
            this->actions.col_actions[j] = j;
        };
    }

    void apply_actions(
        typename Types::Action row_action, 
        typename Types::Action col_action)
    {
        this->current = this->current->access(row_action, col_action)->access(this->transition);
        update_solved_state_payoffs(current);
    }

    void update_solved_state_payoffs(SeedStateNode *current)
    {
        this->row_payoff = this->current->stats.payoff;
        this->col_payoff = 1 - this->row_payoff;
        const int rows = this->current->actions.rows;
        const int cols = this->current->actions.cols;
        for (int i = 0; i < rows; ++i)
        {
            this->row_strategy[i] = this->current->stats.row_strategy[i];
        }
        for (int j = 0; j < cols; ++j)
        {
            this->col_strategy[j] = this->current->stats.col_strategy[j];
        }
    }

    // int count()
    // {
    //     return this->current->count();
    // }

    // Linear::Bimatrix2D<double, size> get_expected_value_matrix()
    // {
    //     return this->current->stats.expected_value;
    // }
};