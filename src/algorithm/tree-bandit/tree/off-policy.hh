#pragma once

#include <types/types.hh>
#include <tree/tree.hh>
#include <algorithm/algorithm.hh>
#include <algorithm/tree-bandit/tree/bandit.hh>

// #include <torch/torch.h>
#include <thread>
#include <chrono>

constexpr int BATTLE_INPUT_DIM = 370;

using namespace std::chrono;

// outcome struct where we only store the policy for the selected action (indices) for either player

template <class Model, class BanditAlgorithm, class _Outcome>
class OffPolicy : public AbstractAlgorithm<Model>
{
public:
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = _Outcome;
    };

    void run(
        size_t learner_iterations,
        size_t actor_iterations,
        typename Types::PRNG &device,
        std::vector<typename Types::State> &states,
        typename Types::Model &model,
        std::vector<MatrixNode<BanditAlgorithm>> &matrix_nodes)
    {

        // Perform batched inference on all trees
        // grab `actor` many samples, inference, update
        // do this `leaner` many times

        for (auto matrix_node : matrix_nodes)
        {
            this->_initialize_stats(learner_iterations * actor_iterations, states[0], model, &matrix_node);
        }

        std::vector<MatrixNode<BanditAlgorithm>> leafs{};

        for (int learner_iteration = 0; learner_iteration < learner_iterations; ++learner_iteration)
        {

            get_leafs(actor_iterations, device, states, model, matrix_nodes, leafs);

            // batched inference on all states? state outputs?
            model.inference();

            // redistribute
            update_leafs(leafs);

        }
    }

    void get_leafs(
        size_t actor_iterations,
        typename Types::PRNG &device,
        std::vector<typename Types::State> &states,
        typename Types::Model &model,
        std::vector<MatrixNode<BanditAlgorithm>> &matrix_nodes,
        std::vector<MatrixNode<BanditAlgorithm>> &leafs)
    {
        for (auto &matrix_node : matrix_nodes)
        {
            auto state = states[0];
            for (size_t actor_iteration = 0; actor_iteration < actor_iterations; ++actor_iteration) {
                auto state_copy = state;
                auto leaf = run_iteration(device, state_copy, model, matrix_node);
                leafs.push_back(leaf);
            }
        }
    }

    void update_leafs(std::vector<MatrixNode<BanditAlgorithm>*> &leafs) {}

protected:
    void _get_empirical_strategies(
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        return static_cast<BanditAlgorithm *>(this)->get_empirical_strategies(
            matrix_node,
            row_strategy,
            col_strategy);
    }

    void _get_empirical_values(
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::Real &row_value,
        typename Types::Real &col_value)
    {
        return static_cast<BanditAlgorithm *>(this)->get_empirical_values(
            matrix_node,
            row_value,
            col_value);
    }

    void _select(
        typename Types::PRNG &device,
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::Outcome &outcome)
    {
        return static_cast<BanditAlgorithm *>(this)->select(
            device,
            matrix_node,
            outcome);
    }

    void _initialize_stats(
        size_t iterations,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *root)
    {
        return static_cast<BanditAlgorithm *>(this)->initialize_stats(
            iterations,
            state,
            model,
            root);
    }

    void _expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *matrix_node)
    {
        state.get_actions();
        matrix_node->is_terminal = state.is_terminal;
        if (state.is_terminal)
        {
            matrix_node->is_terminal = true;
            return;
        }

        matrix_node->row_actions = state.row_actions;
        matrix_node->col_actions = state.col_actions;
        matrix_node->is_expanded = true;

        static_cast<BanditAlgorithm *>(this)->expand(
            state,
            model,
            matrix_node);
    }

    MatrixNode<BanditAlgorithm> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                _select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<BanditAlgorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<BanditAlgorithm> *matrix_node_next = chance_node->access(state.obs, state.prob);

                MatrixNode<BanditAlgorithm> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next);

                // outcome.row_value = matrix_node_leaf->inference.row_value;
                // outcome.col_value = matrix_node_leaf->inference.col_value;
                // _update_matrix_node(matrix_node, outcome);
                // _update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                // this->_expand(state, model, matrix_node);
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }
};