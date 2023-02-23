#pragma once

#include "model/monte_carlo.hh"

#include "seed_state.hh"
#include "grow.hh"

template <int size>
class TreeState : public State<size, int, int>
{
public:
    prng &device;
    SeedState<size> seed;
    MatrixNode<Grow<MonteCarlo<SeedState<size>>>>& root;
    MatrixNode<Grow<MonteCarlo<SeedState<size>>>> *current;

    TreeState(prng &device, MatrixNode<Grow<MonteCarlo<SeedState<size>>>>& root, int depth_bound, int rows, int cols) : State<size, int, int>(device), device(device), seed(SeedState<size>(device, depth_bound, rows, cols)), root(root)
    {
        current = &root;
        Grow<MonteCarlo<SeedState<size>>> session(device);
        session.grow(seed, current);
    }

    typename TreeState::pair_actions_t get_legal_actions()
    {
        typename TreeState::pair_actions_t legal_actions;
        legal_actions.rows = this->current->stats.expected_value.rows;
        legal_actions.cols = this->current->stats.expected_value.cols;
        for (int i = 0; i < legal_actions.rows; ++i)
        {
            legal_actions.actions0[i] = i;
        };
        for (int j = 0; j < legal_actions.cols; ++j)
        {
            legal_actions.actions1[j] = j;
        };
        return legal_actions;
    }

    void get_legal_actions(typename TreeState::pair_actions_t &legal_actions)
    {
        legal_actions.rows = this->current->stats.expected_value.rows;
        legal_actions.cols = this->current->stats.expected_value.cols;
        for (int i = 0; i < legal_actions.rows; ++i)
        {
            legal_actions.actions0[i] = i;
        };
        for (int j = 0; j < legal_actions.cols; ++j)
        {
            legal_actions.actions1[j] = j;
        };
    }

    typename TreeState::transition_data_t apply_actions(typename TreeState::action_t action0, typename TreeState::action_t action1)
    {
        typename TreeState::transition_data_t transition_data(0, Rational(1));
        this->current = this->current->access(action0, action1)->access(transition_data);
        this->payoff0 = this->current->stats.payoff;
        this->payoff1 = 1 - this->payoff0;
        return transition_data;
    }
};