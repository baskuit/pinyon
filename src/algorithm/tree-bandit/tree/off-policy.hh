#pragma once

#include "../../types/types.hh"
#include "../../tree/tree.hh"
#include "../algorithm.hh"

#include <torch/torch.h>
#include <thread>
#include <chrono>

constexpr int BATTLE_INPUT_DIM = 370;

using namespace std::chrono;

// outcome struct where we only store the policy for the selected action (indices) for either player

template <typename Model>
struct ChoicesOutcome
{
    using Real = typename Model::Types::Real;
    ActionIndex row_idx, col_idx;
    Real row_value, col_value;
    Real row_mu, col_mu;
};

// outcome struct where we store the entire policy, for all actions

template <typename Model>
struct PolicyOutcome
{
    using Real = typename Model::Types::Real;
    using VectorReal = typename Model::Types::VectorReal;

    ActionIndex row_idx, col_idx;
    Real row_value, col_value;
    VectorReal row_policy, col_policy;
};

template <class Model, class BanditAlgorithm, class _Outcome>
class BatchedTreeBandit : public AbstractAlgorithm<Model>
{
public:
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = _Outcome;
    };

    size_t batch_size;
    size_t thread_batch_size;
    size_t trees_per_thread;

    size_t threads;

    Torch::Tensor thread_input_tensor = Torch::empty({thread_batch_size, BATTLE_INPUT_DIM});
    Torch::Tensor thread_output_tensor = Torch::empty({thread_batch_size, 1});

    // buffer for obs tensor, policy, score, value etc TRAINING DATA

    void get_threads_started()
    {
    }

    void thread_function(
        uint64_t device_seed,
        std::vector<typename Types::State> &states,
        Model &model,
        std::vector<MatrixNode<BanditAlgorithm>> &roots
    )
    {

        typename Types::PRNG device(thread_seed);

        std::vector<MatrixNode<BanditAlgorithm>> leafs;
        leafs.resize(thread_batch_size);

        const size_t samples_per_tree = thread_batch_size / trees_per_thread;

        while (true)
        {

            // game after game

            int thread_batch_index = 0;

            for (auto &root : roots)
            {

                for (size_t iteration = 0; iteration < samples_per_tree; ++iteration)
                {

                    auto leaf_node = run_iteration(device, state, model, root);
                    // state is now rolled out as well
                    auto row_observation_tensor = model->get_row_obs(state);
                    thread_input_tensor[thread_batch_index] = row_observation_tensor;
                    thread_batch_index++;
                }
            }

            // All nodes observed, thread_batch waiting to go.

            model->get_inference(thread_input_tensor, thread_output_tensor);

            thread_batch_index = 0;

            for (int b = 0; b < thread_batch_size; ++b)
            {

                auto leaf = leafs[b];

                // state? just need actions, do that earlier

                if (leaf.is_expanded)
                {
                    // skip?
                }
                else
                {
                    _expand(state, model, leaf);
                    update_post_facto(leaf);
                }
            }
        };
    }

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
        if (state.is_terminal) {
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

    void update_post_facto(
        MatrixNode<BanditAlgorithm> *matrix_node)
    {
        if (matrix_node == nullptr)
        {
            return;
        }

        if (matrix_node->is_expanded)
        {

        }

        ChanceNode<BanditAlgorithm> *chance_node_parent = matrix_node->parent;

        while (chance_node_parent) {

            

            MatrixNode<BanditAlgorithm> *matrix_node_parent = chance_node_parent->parent;
            matrix_node = matrix_node_parent;
            chance_node_parent = matrix_node->parent;
        }
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