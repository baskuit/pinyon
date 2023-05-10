#pragma once

#include "model/model.hh"
#include "seed-state.hh"
#include "grow.hh"
#include "tree/tree.hh"

#include <memory>

/*

This state is a wrapper for the tree produced by the Grow algorithm

What should template type be? Just model?

*/

template <class Model>
class TreeState : public SolvedState<typename Model::Types::TypeList>
{
    static_assert(std::derived_from<typename Model::Types::State, StateChance<typename Model::Types::TypeList>>,
        "TreeState must be based on State type derived from StateChance");
    static_assert(std::derived_from<Model, DoubleOracleModel<typename Model::Types::State>>,
        "Grow algorithm requires Model must be derived from DoubleOracleModel");

public:
    struct Types : Model::Types
    {
    };

    prng device;
    std::shared_ptr<MatrixNode<Grow<Model>>> tree;
    MatrixNode<Grow<Model>> *current_node;
    typename Types::State::Transition transition;

    TreeState(
        typename Types::State &state,
        Model &model,
        int max_depth=-1) :
            device(state.device)
    {
        this->tree = std::make_shared<MatrixNode<Grow<Model>>>();
        this->current_node = &*tree;
        Grow<Model> session;
        session.max_depth = max_depth;
        session.grow(state, model, current_node);
        update_solved_state_info();
    }

    void get_actions()
    {
        this->actions = current_node->actions;
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        ChanceNode<Grow<Model>> *chance_node = current_node->access(row_action, col_action);
        const int chance_idx = device.sample_pdf(chance_node->stats.chance_strategy);
        typename Types::Observation chance_action = chance_node->stats.chance_actions[chance_idx];
        transition.obs = chance_action;
        current_node = chance_node.access(transition);
        update_solved_state_info();
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action)
    {
        transition.obs = chance_action;
        current_node = current_node->access(row_action, col_action)->access(transition);
        update_solved_state_info();
    }

    void get_payoff_matrix(
        typename Types::MatrixReal &row_payoff_matrix,
        typename Types::MatrixReal &col_payoff_matrix
    ) {
        row_payoff_matrix = current_node->stats.nash_payoff_matrix;
        col_payoff_matrix = row_payoff_matrix * -1 + 1;
    }

private:
    void update_solved_state_info()
    {
        this->is_terminal = current_node->is_terminal; 
        this->row_payoff = current_node->stats.row_payoff;
        this->col_payoff = 1 - this->row_payoff;
        this->row_strategy = current_node->stats.row_solution;
        this->col_strategy = current_node->stats.col_solution; // wasteful for arrays
    }
};