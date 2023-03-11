#pragma once

#include "state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "tree/tree.hh"

// Large uniform tree for testing etc. So called because it spreads out until it can't.

template <int size>
class MoldState : public StateArray<size, int, int, bool>
{
public:
    struct Types : StateArray<size, int, int, bool>::Types
    {
    };
    int depth = 1;

    MoldState(int depth) : depth((depth >= 0) * depth)
    {
        for (int i = 0; i < size; ++i)
        {
            this->actions.row_actions[i] = i;
            this->actions.col_actions[i] = i;
        }
        this->actions.rows = size;
        this->actions.cols = size;
        this->transition.prob = true;
        this->transition.obs = 0;
    }

    void get_actions()
    {
        if (this->depth <= 0)
        {
            this->actions.rows = 0;
            this->actions.cols = 0;
            this->is_terminal = true;
        }
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        --this->depth;
    }
};

class PennyMatching : public StateArray<2, int, int, bool>
{
public:
    struct Types : StateArray<2, int, int, bool>::Types
    {
    };

    void get_actions()
    {
        this->actions.rows = 2;
        this->actions.cols = 2;
        for (int i = 0; i < 2; ++i)
        {
            this->actions.row_actions[i] = i;
            this->actions.col_actions[i] = i;
        }
    }
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action // PennyMatching has a fixed Action type, but nevertheless we can pretend it doesn't too see how to handle things when they get less clear.
    )
    {
        this->transition.prob = true;
        this->transition.obs = 0;
        this->is_terminal = true;
        if (row_action == col_action)
        {
            this->row_payoff = 1.0;
            this->col_payoff = 0.0;
        }
        else
        {
            this->row_payoff = 0.0;
            this->col_payoff = 1.0;
        }
    }
};

class Sucker : public StateArray<2, int, int, bool>
{
public:
    struct Types : StateArray<2, int, int, bool>::Types
    {
    };

    void get_actions()
    {
        this->actions.rows = 2;
        this->actions.cols = 2;
        for (int i = 0; i < 2; ++i)
        {
            this->actions.row_actions[i] = i;
            this->actions.col_actions[i] = i;
        }
    }
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action // PennyMatching has a fixed Action type, but nevertheless we can pretend it doesn't too see how to handle things when they get less clear.
    )
    {
        this->transition.prob = true;
        this->transition.obs = 0;
        this->is_terminal = true;
        if (row_action == col_action)
        {
            if (row_action == 0)
            {
                this->row_payoff = 0.333;
                this->col_payoff = 0.666;
            }
            else
            {
                this->row_payoff = 1.0;
                this->col_payoff = 0.0;
            }
        }
        else
        {
            this->row_payoff = 0.0;
            this->col_payoff = 1.0;
        }
    }
};
// TODO test
template <typename _TypeList>
class BimatrixGame : public DefaultState<_TypeList>
{
    // static_assert(std::derived_from<State, DefaultState<typename State::TypeList>>);

public:
    struct Types : DefaultState<_TypeList>::Types
    {
        using Action = int;
    };

    typename Types::MatrixReal &row_matrix;
    typename Types::MatrixReal &col_matrix;

    BimatrixGame(
        typename Types::MatrixReal &row_matrix,
        typename Types::MatrixReal &col_matrix) : row_matrix(row_matrix), col_matrix(col_matrix) {}

    void get_actions()
    {
        this->actions.rows = row_matrix.rows;
        this->actions.cols = row_matrix.cols;
        for (int i = 0; i < row_matrix.rows; ++i)
        {
            this->actions.row_actions[i] = i;
        }
        for (int j = 0; j < row_matrix.cols; ++j)
        {
            this->actions.col_actions[j] = j;
        }
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        this->transition.prob = true; // hehe
        this->transition.obs = 0;
        this->is_terminal = true;
        this->row_payoff = row_matrix.get(row_action, col_action);
        ;
        this->col_payoff = col_matrix.get(row_action, col_action);
        ;
    }

    void solve(
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        prng device;
        MonteCarloModel<BimatrixGame> model(device);
        Exp3p<MonteCarloModel<BimatrixGame>, TreeBandit> session(device);
        MatrixNode<Exp3p<MonteCarloModel<BimatrixGame>, TreeBandit>> root;
        session.run(10000, *this, model, root);
        session.get_strategies(&root, row_strategy, col_strategy);
    }
};