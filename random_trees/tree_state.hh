#pragma once

#include "model/monte_carlo.hh"

#include "seed_state.hh"
#include "grow.hh"

template <int size>
class TreeState : public State<size, int, int> {

    prng& device;
    SeedState<size> seed;
    MatrixNode<MonteCarlo<SeedState<size>>>* root;
    MatrixNode<MonteCarlo<SeedState<size>>>* current;


    typename TreeState::pair_actions_t get_legal_actions()
    {
        typename TreeState::pairs_t legal_actions;
        legal_actions.rows = this->rows;
        legal_actions.cols = this->cols;
        for (int i = 0; i < this->rows; ++i)
        {
            legal_actions.actions0[i] = i;
        };
        for (int j = 0; j < this->cols; ++j)
        {
            legal_actions.actions1[j] = j;
        };
        return legal_actions;
    }

    void get_legal_actions(typename TreeState::pair_actions_t &legal_actions)
    {
        legal_actions.rows = this->rows;
        legal_actions.cols = this->cols;
        for (int i = 0; i < this->rows; ++i)
        {
            legal_actions.actions0[i] = i;
        };
        for (int j = 0; j < this->cols; ++j)
        {
            legal_actions.actions1[j] = j;
        };
    }

    typename TreeState::transition_data_t apply_actions(typename TreeState::action_t action0, typename TreeState::action_t action1)
    {
        if (--this->depth_bound == 0)
        {
            this->payoff0 = this->device.random_int(2);
            this->payoff1 = 1.0 - this->payoff0;
            this->rows = 0;
            this->cols = 0;
        }
        typename TreeState::transition_data_t transition_data(0, Rational(1));
        return transition_data;
    }
};