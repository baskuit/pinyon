#pragma once

#include <libsurskit/math.hh>
#include <model/model.hh>
#include <state/random-tree.hh>
#include <algorithm/solver/full-traversal.hh>
#include <tree/tree.hh>

#include <memory>
#include <numeric>

/*

This state is a wrapper for the tree produced by the FullTraversal algorithm

Its typelist is the typelist of the inner State, Model type

*/

template <IsValueModelTypes Types>
    requires IsChanceStateTypes<Types>
struct TraversedState : Types::TypeList
{
    template <IsNodeTypes NodePair>
    class State_;

    using State = State_<DefaultNodes<Types, typename FullTraversal<Types>::MatrixStats, typename FullTraversal<Types>::ChanceStats>>;

    // This hidden template impl allows for type hints
    template <IsNodeTypes NodePair>
    class State_ : PerfectInfoState<typename Types::TypeList>
    {
    public:

        typename Types::PRNG device{};
        std::shared_ptr<typename NodePair::MatrixNode> tree;
        typename NodePair::MatrixNode *current_node;
        double chance_p;

        State_(
            Types::State &state,
            Types::Model &model,
            int max_depth = -1)
        {
            this->tree = std::make_shared<typename NodePair::MatrixNode>();
            this->current_node = &*tree;
            typename FullTraversal<Types>::Search session{max_depth};
            session.run(state, model, current_node);
            update_perfect_info_state();
        }

        void get_actions()
        {
            this->row_actions = current_node->row_actions;
            this->col_actions = current_node->col_actions;
            sort_actions(this->row_actions, this->col_actions, this->row_strategy, this->col_strategy);
                    typename NodePair::MatrixNode *node;
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
            chance_p = device.uniform();
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action)
        {
            int row_idx = std::find(this->row_actions.begin(), this->row_actions.end(), row_action) - this->row_actions.begin();
            int col_idx = std::find(this->col_actions.begin(), this->col_actions.end(), col_action) - this->col_actions.begin();
            typename NodePair::ChanceNode *chance_node = current_node->access(row_idx, col_idx);
            auto &chance_strategy = chance_node->stats.chance_strategy;

            int chance_idx = 0;
            while (chance_p > 0 && chance_idx + 1 < chance_strategy.size())
            {
                chance_p -= static_cast<double>(chance_strategy[chance_idx]);
                ++chance_idx;
            }

            const typename Types::Obs chance_action = chance_node->stats.chance_actions[chance_idx];
            current_node = chance_node->access(chance_action);
            update_perfect_info_state();
        }

        void get_chance_actions(
            std::vector<typename Types::Obs> &chance_actions,
            Types::Action row_action,
            Types::Action col_action) const
        {
            int row_idx = std::find(this->row_actions.begin(), this->row_actions.end(), row_action) - this->row_actions.begin();
            int col_idx = std::find(this->col_actions.begin(), this->col_actions.end(), col_action) - this->col_actions.begin();
            typename NodePair::ChanceNode *chance_node = current_node->access(row_idx, col_idx);
            chance_actions = chance_node->stats.chance_actions;
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action,
            Types::Obs chance_action)
        {
            int row_idx = std::find(this->row_actions.begin(), this->row_actions.end(), row_action) - this->row_actions.begin();
            int col_idx = std::find(this->col_actions.begin(), this->col_actions.end(), col_action) - this->col_actions.begin();
            current_node = current_node->access(row_idx, col_idx)->access(chance_action);
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