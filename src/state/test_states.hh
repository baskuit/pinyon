#pragma once

#include "state.hh"
#include "model/model.hh"
#include "algorithm/exp3p.hh"
#include "tree/tree.hh"

/*
 Large uniform tree for testing etc. So called because it grows until it can't.
*/

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
        typename Types::Action col_action
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

/*
"Sucker Punch" game from Into-To-Pokemon, with PP=3
*/

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

/*
One shot matrix game from row and column player payoff matrices
*/

template <typename TypeList>
class BimatrixGame : public DefaultState<TypeList>
{
    static_assert(std::derived_from<TypeList, AbstractTypeList>);

public:
    struct Types : DefaultState<TypeList>::Types
    {
        using Action = int;
    };

    typename Types::MatrixReal &row_payoff_matrix;
    typename Types::MatrixReal &col_payoff_matrix;

    BimatrixGame(
        typename Types::MatrixReal &row_payoff_matrix,
        typename Types::MatrixReal &col_payoff_matrix) : row_payoff_matrix(row_payoff_matrix), col_payoff_matrix(col_payoff_matrix) {}

    void get_actions()
    {
        this->actions.rows = row_payoff_matrix.rows;
        this->actions.cols = col_payoff_matrix.cols;
        for (int i = 0; i < row_payoff_matrix.rows; ++i)
        {
            this->actions.row_actions[i] = i;
        }
        for (int j = 0; j < col_payoff_matrix.cols; ++j)
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
        this->row_payoff = row_payoff_matrix.get(row_action, col_action);
        this->col_payoff = col_payoff_matrix.get(row_action, col_action);
    }

    void solve(
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        prng device;
        MonteCarloModel<BimatrixGame> model(device);
        Exp3p<MonteCarloModel<BimatrixGame>, TreeBandit> session;
        MatrixNode<Exp3p<MonteCarloModel<BimatrixGame>, TreeBandit>> root;
        session.run(10000, device, *this, model, root);
        session.get_strategies(&root, row_strategy, col_strategy);
    }
};

/*
Vector version of Sucker for testing
*/

class SuckerVector : public StateVector<int, int, bool>
{
public:
    struct Types : StateVector<int, int, bool>::Types
    {
    };

    void get_actions()
    {
        this->actions.rows = 2;
        this->actions.cols = 2;
        this->actions.row_actions.clear();
        this->actions.col_actions.clear();
        for (int i = 0; i < 2; ++i)
        {
            this->actions.row_actions.push_back(i);
            this->actions.col_actions.push_back(i);
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

template <int size>
class MoldStateVector : public StateVector<int, int, bool>
{
public:
    struct Types : StateVector<int, int, bool>::Types
    {
    };
    int depth = 1;

    MoldStateVector(int depth) : depth((depth >= 0) * depth)
    {
        this->actions.row_actions.clear();
        this->actions.col_actions.clear();
        for (int i = 0; i < size; ++i)
        {
            this->actions.row_actions.push_back(i);
            this->actions.col_actions.push_back(i);
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
