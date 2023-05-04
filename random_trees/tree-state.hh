#pragma once

#include "model/model.hh"
#include "seed-state.hh"
#include "grow.hh"

#include <memory>

/*

This state is a wrapper for the tree produced by the Grow algorithm

What should template type be? Just model?

*/

template <class Model>
class TreeState : public SolvedState<Model::Types::TypeList>
{
    static_assert(std::derived_from<Model::Types::State, StateChance<Model::Types::TypeList>>,
        "TreeState must be based on State type derived from StateChance");
    static_assert(std::derived_from<Model, DoubleOracleModel>,
        "Grow algorithm requires Model must be derived from DoubleOracleModel");

public:
    using Node = MatrixNode<Grow<Model>>;

    struct Types : Model::Types
    {
    };

    std::shared_ptr<Node> tree;
    Node *current;

    TreeState(
        typename Types::State &state,
        Model &model,
        int max_depth)
    {
        this->tree = std::make_shared<Node>();
        this->current = &*tree;
        Grow<Model> session;
        session.max_depth = max_depth;
        session.grow(state, model, current);
        update_solved_state_info(current);
    }

    void get_actions()
    {
        this->actions = current->actions;
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        this->current = this->current->access(row_action, col_action)->access(transition); // TODO how to make transition like the OG state without a state?
        update_solved_state_info(this->current);
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action)
    {
        this->transition.obs = chance_action;
        this->current = this->current->access(row_action, col_action)->access(this->transition);
        update_solved_state_info(this->current);
    }

private:
    void update_solved_state_info(Node *current)
    {
        this->is_terminal = this->current->is_terminal; 
        // this->row_payoff = this->current->stats.payoff;
        // this->col_payoff = 1 - this->row_payoff;
        // const int rows = this->current->actions.rows;
        // const int cols = this->current->actions.cols;
        // this->row_strategy.fill(rows);
        // this->col_strategy.fill(cols);
        // for (int i = 0; i < rows; ++i)
        // {
        //     this->row_strategy[i] = this->current->stats.row_strategy[i];
        // }
        // for (int j = 0; j < cols; ++j)
        // {
        //     this->col_strategy[j] = this->current->stats.col_strategy[j];
        // }
    }
};