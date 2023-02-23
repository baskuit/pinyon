#pragma once

#include "model/monte_carlo.hh"

#include "seed_state.hh"
#include "grow.hh"

template <int size>
class TreeState : public SolvedState<size, int, int>
{
public:

    using SeedStateNode = MatrixNode<Grow<MonteCarlo<SeedState<size>>>>;

    SeedState<size> seed;
    std::shared_ptr<SeedStateNode> root;
    SeedStateNode *current;
    // Trying out make shared to keep copy assignment but also have automatic memory management

    TreeState(prng &device, int depth_bound, int rows, int cols) : 
        SolvedState<size, int, int>(device), seed(SeedState<size>(device, depth_bound, rows, cols))
    {
        this->root = std::make_shared<SeedStateNode>();
        this->current = &*root;
        Grow<MonteCarlo<SeedState<size>>> session(device);
        session.grow(seed, current);
        update_solved_state(current);
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
        // updating SolvedState members
        update_solved_state(current);
        return transition_data;
    }

    void update_solved_state (SeedStateNode* current) {
        this->payoff0 = this->current->stats.payoff;
        this->payoff1 = 1 - this->payoff0;
        this->rows = this->current->legal_actions.rows;
        this->cols = this->current->legal_actions.cols;
        for (int i = 0; i < this->rows; ++i) {
            this->strategy0[i] = this->current->stats.strategy0[i];
        }
        for (int j = 0; j < this->cols; ++j) {
            this->strategy1[j] = this->current->stats.strategy1[j];
        }
    }

    int count () {
        return this->current->count();
    }

    Linear::Bimatrix2D<double, size> get_expected_value_matrix () {
        return this->current->stats.expected_value;
    }
};