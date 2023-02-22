#include "state.hh"
#include "../tree/node.hh"
#include "../search/matrix_ucb.hh"
#include "../model/monte_carlo.hh"

/*
SeedState contains just enough info to pseudo-randomly expand to a recursively solved game tree.

The actual state representing the solved tree will be the TreeState object,
which is simply a wrapper the MatrixNode/ChanceNode tree created by the Grow algorithm on the SeedState.
*/

template <int size>
class SeedState : public State<int, int, size>
{
    int depth_bound = 0;
    int rows = size;
    int cols = size;

    SeedState(prng &device, int depth_bound, int rows, int cols) : State<size, int, int>(device), depth_bound(depth_bound), rows(rows), cols(cols) {}

    typename SeedState::pair_actions_t get_legal_actions()
    {
        typename SeedState::pairs_t legal_actions;
        legal_actions.rows = this->rows;
        legal_actions.cols = this->cols;
        for (int i = 0; i < this->rows; ++i)
        {
            legal_actions.actions0[i] = i;
        };
        for (int j = 0; i < this->cols; ++j)
        {
            legal_actions.actions1[j] = j;
        };
        return legal_actions;
    }

    void get_legal_actions(typename SeedState::pair_actions_t &legal_actions)
    {
        legal_actions.rows = this->rows;
        legal_actions.cols = this->cols;
        for (int i = 0; i < this->rows; ++i)
        {
            legal_actions.actions0[i] = i;
        };
        for (int j = 0; i < this->cols; ++j)
        {
            legal_actions.actions1[j] = j;
        };
    }

    typename SeedState::transition_data_t apply_actions(typename SeedState::action_t action0, typename SeedState::action_t action1)
    {
        if (--this->depth_bound == 0)
        {
            this->payoff0 = this->device.random_int(2);
            this->payoff1 = 1.0 - this->payoff0;
            this->rows = 0;
            this->cols = 0;
        }
        typename SeedState::transition_data_t transition_data(0, Rational(1));
        return transition_data;
    }
};

template <int size>
class TreeState : public State<size, int, int>
{

    MatrixNode<MatrixUCB<MonteCarlo<SeedState<size>>>> *root;
    MatrixNode<MatrixUCB<MonteCarlo<SeedState<size>>>> *current;
};