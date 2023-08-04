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
requires IsChanceStateTypes<Types>
struct TraversedState : Types::TypeList
{

    class State : PerfectInfoState<typename Types::TypeList>
    {
    public:
        using MatrixNode = typename DefaultNodes<Types, typename FullTraversal<Types>::MatrixStats, typename FullTraversal<Types>::ChanceStats>::MatrixNode;
        using ChanceNode = typename DefaultNodes<Types, typename FullTraversal<Types>::MatrixStats, typename FullTraversal<Types>::ChanceStats>::ChanceNode;

        typename Types::State base_state;
        std::shared_ptr<MatrixNode> tree;
        MatrixNode *current_node;

        State(
            Types::State &state,
            Types::Model &model,
            int max_depth = -1) : base_state{state}
        {
            this->tree = std::make_shared<MatrixNode>();
            this->current_node = &*tree;
            FullTraversal<Types> session{max_depth};
            session.run(state, model, current_node);
            update_perfect_info_state();
        }

        void get_actions()
        {
            this->row_actions = current_node->row_actions;
            this->col_actions = current_node->col_actions;
            sort_actions(this->row_actions, this->col_actions, this->row_strategy, this->col_strategy);
        }

        void get_actions(
            Types::VectorAction &row_actions,
            Types::VectorAction &col_actions) const
        {
            this->row_actions = current_node->row_actions;
            this->col_actions = current_node->col_actions;
            sort_actions(row_actions, col_actions, this->row_strategy, this->col_strategy);
        }

        void randomize_transition(
            typename Types::PRNG &device)
        {
            base_state.randomize_transition(device);
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action)
        {
            ActionIndex row_idx = std::find(this->row_actions.begin(), this->row_actions.end(), row_action) - this->row_actions.begin();
            ActionIndex col_idx = std::find(this->col_actions.begin(), this->col_actions.end(), col_action) - this->col_actions.begin();
            ChanceNode *chance_node = current_node->access(row_idx, col_idx);
            const size_t chance_idx = this->seed % chance_node->stats.chance_actions.size();
            typename Types::Obs chance_action = chance_node->stats.chance_actions[chance_idx];
            current_node = chance_node->access(chance_action);
            update_perfect_info_state();
        }

        void get_chance_actions(
            std::vector<typename Types::Obs> &chance_actions,
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
            Types::Action col_action,
            Types::Obs chance_action)
        {
            base_state.apply_actions(row_action, col_action, chance_action);
            current_node = current_node->access(row_action, col_action)->access(chance_action);
            update_perfect_info_state();
        }

        void get_strategies(
            Types::VectorReal &row_strategy,
            Types::VectorReal &col_strategy) const
        {
            row_strategy = current_node->stats.row_solution;
            col_strategy = current_node->stats.col_solution;
        }

    private:
        void update_perfect_info_state()
        {
            this->terminal = current_node->is_terminal();
            this->payoff = current_node->stats.payoff;
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
};