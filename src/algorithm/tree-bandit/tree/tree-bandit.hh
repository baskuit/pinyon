#pragma once

#include <types/types.hh>
#include <algorithm/algorithm.hh>

#include <chrono>

template <
    class BanditAlgorithm,
    template <class> class MNode,
    template <class> class CNode,
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
        using MatrixNode = MNode<TreeBandit>;
        using ChanceNode = CNode<TreeBandit>;
    };

    struct MatrixStats : BanditAlgorithm::MatrixStats
    {
    };
    struct ChanceStats : BanditAlgorithm::ChanceStats
    {
    };

    using BanditAlgorithm::BanditAlgorithm;

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> &matrix_node)
    {
        // this->initialize_stats(iterations, state, model, matrix_node.stats);
        typename Types::ModelOutput inference;
        for (size_t iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device);
            this->run_iteration(device, state_copy, model, &matrix_node, inference);
        }
    }

    void run_for_duration(
        size_t duration_us,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> &matrix_node)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        typename Types::ModelOutput inference;
        while (duration.count() < duration_us)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device);
            this->run_iteration(device, state_copy, model, &matrix_node, inference);

            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
    }

protected:
    MatrixNode<TreeBandit> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> *matrix_node,
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

            ChanceNode<TreeBandit> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
            MatrixNode<TreeBandit> *matrix_node_next = chance_node->access(state.obs);

            MatrixNode<TreeBandit> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

            outcome.value = inference.value;
            this->update_matrix_stats(matrix_node->stats, outcome);
            this->update_chance_stats(chance_node->stats, outcome);
            return matrix_node_leaf;
        }
        else
        {
            if constexpr (MatrixNode<TreeBandit>::STORES_VALUE) {
                matrix_node->get_value(inference.value);
            } else {
                inference.value = state.payoff;
            }  
            return matrix_node;
        }
    }

    MatrixNode<TreeBandit> *run_iteration_average(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> *matrix_node,
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

            ChanceNode<TreeBandit> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
            MatrixNode<TreeBandit> *matrix_node_next = chance_node->access(state.obs);

            MatrixNode<TreeBandit> *matrix_node_leaf = run_iteration_average(device, state, model, matrix_node_next, inference);

            this->get_empirical_value(matrix_node_next->stats, outcome.value);
            // TODO use chance node? Breaks if matrix_node_next is terminal?
            this->update_matrix_stats(matrix_node->stats, outcome);
            this->update_chance_stats(chance_node->stats, outcome);
            return matrix_node_leaf;
        }
        else
        {
            if constexpr (MatrixNode<TreeBandit>::STORES_VALUE) {
                matrix_node->get_value(inference.value);
            } else {
                inference.value = state.payoff;
            }  
            return matrix_node;
        }
    }
};