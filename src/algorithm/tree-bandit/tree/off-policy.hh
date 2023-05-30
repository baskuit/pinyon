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

    using ActorPolicies = std::vector<std::vector<double>>;

    struct Info {

        std::vector<double> actor_policy;
        MatrixNode<BanditAlgorithm>* leaf;


    };


    void run(
        size_t learner_iterations,
        size_t actor_iterations_per,
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
            this->_initialize_stats(learner_iterations * actor_iterations_per, states[0], model, &matrix_node);
        }

        std::vector<MatrixNode<BanditAlgorithm>> leafs{};
        typename Types::ModelBatchInput input;   // vector of states
        typename Types::ModelBatchOutput output; // vector of inferences/single output
        ActorPolicies actor_policies;

        for (int learner_iteration = 0; learner_iteration < learner_iterations; ++learner_iteration)
        {

            get_leafs(leafs, input, actor_iterations_per, device, states, model, matrix_nodes);

            // batched inference on all states? state outputs?
            model.get_inference(input, output); // append inference structs

            // redistribute
            update_leafs(leafs, output);
        }
    }

    void get_leafs(
        std::vector<MatrixNode<BanditAlgorithm>> &leafs,
        typename Types::ModelBatchInput &input,
        size_t actor_iterations_per,
        typename Types::PRNG &device,
        std::vector<typename Types::State> &states,
        typename Types::Model &model,
        std::vector<MatrixNode<BanditAlgorithm>> &matrix_nodes)
    {

        int state_index = 0;
        for (auto &matrix_node : matrix_nodes)
        {
            auto state = states[state_index];
            for (size_t actor_iteration = 0; actor_iteration < actor_iterations_per; ++actor_iteration)
            {
                auto state_copy = state;
                MatrixNode<BanditAlgorithm>* leaf = get_single_leaf(device, state_copy, model, matrix_node);
                model.add_to_batch_input(state_copy, input);
                leafs.push_back(leaf);
            }
            ++state_index;
        }
    }

    void update_leafs(
        std::vector<MatrixNode<BanditAlgorithm> *> &leafs,
        typename Types::ModelBatchOutput &output)
    {
        for (auto leaf : leafs) {
            update_leaf
        }
    }

    void backtrack (
        MatrixNode<BanditAlgorithm> *matrix_node,
        std::vector<double> &actor_policy,
        typename Types::ModelOutput &ouput,
    ) {
        
        typename Types::Outcome outcome;

        for (auto mu : actor_policy) {

            outcome.row_value = output.row_value;
            outcome.col_value = output.col_value;

            ChanceNode<BanditAlgorithm> *selected_chance_node = matrix_node->parent;
            MatrixNode<BanditAlgorithm> *selecting_matrix_node = selected_chance_node->parent;

            ActionIndex row_idx = selected_chance_node->row_idx;
            ActionIndex col_idx = selected_chance_node->col_idx;

            double pi = _get_policy(selecting_matrix_node, row_idx, col_idx);
            double importance_weight = pi / mu;

            _update_matrix_node(
                selecting_matrix_node,
                output,
                importance_weight
            );
            _update_chance_node(
                selected_chance_node,
                outcome,
                importance_weight
            );

            matrix_node = selecting_matrix_node;
        }
    }

protected:
    double _get_joint_policy (
        MatrixNode<BanditAlgorithm> *matrix_node
    ) {
        return 1;
        return static_cast<BanditAlgorithm *>(this)->get_joint_policy(
            matrix_node);
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

    MatrixNode<BanditAlgorithm> *get_single_leaf(
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

                MatrixNode<BanditAlgorithm> *matrix_node_leaf = get_single_leaf(device, state, model, matrix_node_next);
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

    void _update_matrix_node(
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::Outcome &outcome,
        double learning_rate = 1)
    {
        return static_cast<BanditAlgorithm *>(this)->update_matrix_node(
            matrix_node,
            outcome,
            learning_rate);
    }

    void _update_chance_node(
        ChanceNode<BanditAlgorithm> *chance_node,
        typename Types::Outcome &outcome,
        double learning_rate = 1)
    {
        return static_cast<BanditAlgorithm *>(this)->update_chance_node(
            chance_node,
            outcome,
            learning_rate);
    }
};