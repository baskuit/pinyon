#pragma once

#include <types/types.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

#include <chrono>

template <
    class BanditAlgorithm,
    class NodePair=DefaultNodes,
    bool return_if_expand = true>
class TreeBandit : public BanditAlgorithm
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : BanditAlgorithm::Types
    {
        using MatrixStats = TreeBandit::MatrixStats;
        using ChanceStats = TreeBandit::ChanceStats;
        using MatrixNode = typename NodePair::template MNode<TreeBandit>;
        using ChanceNode = typename NodePair::template CNode<TreeBandit>;
    };

    struct MatrixStats : BanditAlgorithm::MatrixStats
    {
    };
    struct ChanceStats : BanditAlgorithm::ChanceStats
    {
    };

    using BanditAlgorithm::BanditAlgorithm;

    TreeBandit(BanditAlgorithm &base) : BanditAlgorithm{base} {}

    size_t run(
        size_t duration_ms,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixNode &matrix_node)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        typename Types::ModelOutput inference;
        size_t iterations = 0;
        for (; duration.count() < duration_ms; ++iterations)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device);
            this->run_iteration(device, state_copy, model, &matrix_node, inference);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        }
        return iterations;
    }

    size_t run_for_iterations(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixNode &matrix_node)
    {
        auto start = std::chrono::high_resolution_clock::now();
        typename Types::ModelOutput inference;
        for (size_t iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device);
            this->run_iteration(device, state_copy, model, &matrix_node, inference);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return duration.count();
    }

protected:
    typename Types::MatrixNode *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixNode *matrix_node,
        typename Types::ModelOutput &inference)
    {
        if (!matrix_node->is_terminal())
        {
            if (!matrix_node->is_expanded())
            {
                state.get_actions();
                matrix_node->expand(state);
                model.get_inference(state, inference);
                this->expand(state, matrix_node->stats, inference);

                if constexpr (return_if_expand)
                {
                    return matrix_node;
                }

                // TODO do we still have to return if it expands and finds its terminal? Is that what caused that bug?
            }

            typename Types::Outcome outcome;
            this->select(device, matrix_node->stats, outcome);

            matrix_node->apply_actions(state, outcome.row_idx, outcome.col_idx);

            typename Types::ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
            typename Types::MatrixNode *matrix_node_next = chance_node->access(state.obs);

            typename Types::MatrixNode *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

            outcome.value = inference.value;
            this->update_matrix_stats(matrix_node->stats, outcome);
            this->update_chance_stats(chance_node->stats, outcome);
            return matrix_node_leaf;
        }
        else
        {
            if constexpr (Types::MatrixNode::STORES_VALUE)
            {
                matrix_node->get_value(inference.value);
            }
            else
            {
                inference.value = state.payoff;
            }
            return matrix_node;
        }
    }

    typename Types::MatrixNode *run_iteration_average(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixNode *matrix_node,
        typename Types::ModelOutput &inference)
    {
        if (!matrix_node->is_terminal())
        {
            if (!matrix_node->is_expanded())
            {
                state.get_actions();
                matrix_node->expand(state);
                model.get_inference(state, inference);
                this->expand(state, matrix_node->stats, inference);

                if constexpr (return_if_expand)
                {
                    return matrix_node;
                }
            }

            typename Types::Outcome outcome;
            this->select(device, matrix_node->stats, outcome);

            matrix_node->apply_actions(state, outcome.row_idx, outcome.col_idx);

            typename Types::ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
            typename Types::MatrixNode *matrix_node_next = chance_node->access(state.obs);

            typename Types::MatrixNode *matrix_node_leaf = run_iteration_average(device, state, model, matrix_node_next, inference);

            this->get_empirical_value(matrix_node_next->stats, outcome.value);
            // TODO use chance node? Breaks if matrix_node_next is terminal?
            this->update_matrix_stats(matrix_node->stats, outcome);
            this->update_chance_stats(chance_node->stats, outcome);
            return matrix_node_leaf;
        }
        else
        {
            if constexpr (Types::MatrixNode::STORES_VALUE)
            {
                matrix_node->get_value(inference.value);
            }
            else
            {
                inference.value = state.payoff;
            }
            return matrix_node;
        }
    }
};