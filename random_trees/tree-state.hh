#pragma once

#include "model/model.hh"
#include "seed-state.hh"
#include "grow.hh"

#include <memory>

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

    TreeState(prng &device, int depth_bound, int rows, int cols, int (*depth_bound_func)(prng &, int), int (*actions_func)(prng &, int)) : 
        seed(SeedState<size>(device, depth_bound, rows, cols, depth_bound_func, actions_func))
    {
        this->root = std::make_shared<SeedStateNode>();
        this->current = &*root;
        Grow<MonteCarloModel<SeedState<size>>> session;
        session.grow(device, seed, current);
        update_solved_state_payoffs(current);
        this->transition = seed.transition; // total hack
    }

    void get_actions()
    {
        this->actions.rows = this->current->stats.expected_value.rows;
        this->actions.cols = this->current->stats.expected_value.cols;
        this->actions.row_actions.fill(this->actions.rows);
        this->actions.col_actions.fill(this->actions.cols);
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
        typename SeedState<size>::Transition transition;
        transition.obs = 0;
        transition.prob = 1;
        this->current = this->current->access(row_action, col_action)->access(transition);
        this->is_terminal = this->current->is_terminal; 
        update_solved_state_payoffs(this->current);
    }

private:
    void update_solved_state_payoffs(SeedStateNode *current)
    {
        this->row_payoff = this->current->stats.payoff;
        this->col_payoff = 1 - this->row_payoff;
        const int rows = this->current->actions.rows;
        const int cols = this->current->actions.cols;
        this->row_strategy.fill(rows);
        this->col_strategy.fill(cols);
        for (int i = 0; i < rows; ++i)
        {
            this->row_strategy[i] = this->current->stats.row_strategy[i];
        }
        for (int j = 0; j < cols; ++j)
        {
            this->col_strategy[j] = this->current->stats.col_strategy[j];
        }
    }
};