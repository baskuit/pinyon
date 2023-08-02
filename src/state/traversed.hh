#pragma once

#include <model/model.hh>
#include <state/random-tree.hh>
#include <algorithm/solver/full-traversal.hh>
#include <tree/tree.hh>

#include <memory>
#include <numeric>

/*

This state is a wrapper for the tree produced by the FullTraversal algorithm

*/

template <IsValueModelTypes Types>
class TraversedState : PerfectInfoState<Types>
{
public:
    using MatrixNode = typename DefaultNodes<FullTraversal<Types>>::MatrixNode;
    using ChanceNode = typename DefaultNodes<FullTraversal<Types>>::ChanceNode;

    struct T : Types {
        using State = TraversedState;
    };

    std::shared_ptr<MatrixNode> tree;
    MatrixNode *current_node;

    TraversedState(
        Types::State &state,
        Types::Model &model,
        int max_depth = -1)
    {
        this->tree = std::make_shared<MatrixNode>();
        this->current_node = &*tree;
        FullTraversal<Types> session{max_depth};
        session.run(state, model, current_node);
        update_solved_state_info();
    }

    void get_actions()
    {
        this->row_actions = current_node->row_actions;
        this->col_actions = current_node->col_actions;
        sort_actions(this->row_actions, this->col_actions, this->row_strategy, this->col_strategy);
        // return actions in order of quality for alphabeta testing
    }

    void get_actions(
        Types::VectorAction &row_actions,
        Types::VectorAction &col_actions)
    {
        this->row_actions = current_node->row_actions;
        this->col_actions = current_node->col_actions;
        sort_actions(row_actions, col_actions, this->row_strategy, this->col_strategy);
    }

    void get_chance_actions(
        std::vector<typename Types::Observation> &chance_actions,
        Types::Action row_action,
        Types::Action col_action) const
    {
        int row_idx = std::find(this->actions.row_actions.begin(), this->actions.row_actions.end(), row_action) - this->actions.row_actions.begin();
        int col_idx = std::find(this->actions.col_actions.begin(), this->actions.col_actions.end(), col_action) - this->actions.col_actions.begin();
        ChanceNode *chance_node = current_node->access(row_idx, col_idx);
        chance_actions = chance_node->stats.chance_actions;
    }

    void apply_actions(
        Types::Action row_action,
        Types::Action col_action)
    {
        ActionIndex row_idx = std::find(this->row_actions.begin(), this->row_actions.end(), row_action) - this->row_actions.begin();
        ActionIndex col_idx = std::find(this->col_actions.begin(), this->col_actions.end(), col_action) - this->col_actions.begin();
        ChanceNode *chance_node = current_node->access(row_idx, col_idx);
        const size_t chance_idx = this->seed % chance_node->stats.chance_actions.size();
        typename Types::Observation chance_action = chance_node->stats.chance_actions[chance_idx];
        current_node = chance_node->access(chance_action, 0);
        update_solved_state_info();
    }

    void apply_actions(
        Types::Action row_action,
        Types::Action col_action,
        Types::Observation chance_action)
    {
        current_node = current_node->access(row_action, col_action)->access(chance_action, 0);
        update_solved_state_info();
    }

    void get_payoff_matrix(
        Types::MatrixValue &payoff_matrix) const
    {
        payoff_matrix = current_node->stats.nash_payoff_matrix;
    }

private:
    void update_solved_state_info()
    {
        this->terminal = current_node->is_terminal();
        this->payoff = current_node->stats.payoff;
        this->row_strategy = current_node->stats.row_solution;
        this->col_strategy = current_node->stats.col_solution;
    }

    void sort_actions(
        Types::VectorAction &row_actions,
        Types::VectorAction &col_actions,
        Types::VectorReal &row_strategy,
        Types::VectorReal &col_strategy) const
    {
        // sorts row_actions, col_actions by solution probability
        // this way the first action is the 'best' for alphabeta
        typename Types::VectorAction actions;
        typename Types::VectorInt indices;

        std::iota(indices.begin(), indices.begin() + row_actions.size(), 1);
        std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b)
                  { return row_strategy[a] > row_strategy[b]; });
        std::transform(indices.begin(), indices.end(), actions.begin(), [&](size_t i)
                       { return row_actions[i]; });
        row_actions = actions;

        std::iota(indices.begin(), indices.begin() + col_actions.size(), 1);
        std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b)
                  { return col_strategy[a] > col_strategy[b]; });
        std::transform(indices.begin(), indices.end(), actions.begin(), [&](size_t i)
                       { return actions.col_actions[i]; });
        col_actions = actions; // TODO could be optimized
    }
};