#pragma once

#include <types/types.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

#include <chrono>

template <
    CONCEPT(IsBanditAlgorithmTypes, Types),
    template <typename...> typename NodePair = DefaultNodes,
    bool return_if_expand = true>
    requires IsNodeTypes<NodePair<Types, typename Types::MatrixStats, typename Types::ChanceStats>>
// will auto complete all all this ::MatrixNode but not the alias decl :(
struct TreeBandit : Types
{
    using MatrixNode = NodePair<Types, typename Types::MatrixStats, typename Types::ChanceStats>::MatrixNode;
    using ChanceNode = NodePair<Types, typename Types::MatrixStats, typename Types::ChanceStats>::ChanceNode;
    class Search : public Types::BanditAlgorithm
    {
    public:
        using Types::BanditAlgorithm::BanditAlgorithm;

        Search(typename Types::BanditAlgorithm &base) : Types::BanditAlgorithm{base} {}

        size_t run(
            size_t duration_ms,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model,
            MatrixNode &matrix_node) const
        {
            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            typename Types::ModelOutput inference;
            size_t iterations = 0;
            for (; duration.count() < duration_ms; ++iterations)
            {
                typename Types::State state_copy = state;
                state_copy.randomize_transition(device);
                this->run_iteration(device, state_copy, model, &matrix_node, inference);
                end = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            }
            return iterations;
        }

        size_t run_for_iterations(
            const size_t iterations,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model,
            MatrixNode &matrix_node) const
        {
            auto start = std::chrono::high_resolution_clock::now();
            typename Types::ModelOutput inference;
            for (size_t iteration = 0; iteration < iterations; ++iteration)
            {
                typename Types::State state_copy = state;
                state_copy.randomize_transition(device);
                this->run_iteration(device, state_copy, model, &matrix_node, inference);
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return duration.count();
        }

    protected:
        MatrixNode *run_iteration(
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            MatrixNode *matrix_node,
            Types::ModelOutput &inference) const
        {
            if (!matrix_node->is_terminal())
            {
                if (!matrix_node->is_expanded())
                {
                    if (state.is_terminal())
                    {
                        matrix_node->set_terminal();
                        inference.value = state.payoff;
                    }
                    else
                    {
                        state.get_actions();
                        model.get_inference(state, inference);

                        matrix_node->expand(state);
                        this->expand(state, matrix_node->stats, inference);
                    }
                    if constexpr (return_if_expand)
                    {
                        return matrix_node;
                    }
                }

                typename Types::Outcome outcome;
                this->select(device, matrix_node->stats, outcome);

                matrix_node->apply_actions(state, outcome.row_idx, outcome.col_idx);

                ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode *matrix_node_next = chance_node->access(state.obs);

                MatrixNode *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                this->update_matrix_stats(matrix_node->stats, outcome);
                this->update_chance_stats(chance_node->stats, outcome);
                return matrix_node_leaf;
            }
            else
            {
                if constexpr (MatrixNode::STORES_VALUE)
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

        MatrixNode *run_iteration_average(
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            MatrixNode *matrix_node,
            Types::ModelOutput &inference) const
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

                ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode *matrix_node_next = chance_node->access(state.obs);

                MatrixNode *matrix_node_leaf = run_iteration_average(device, state, model, matrix_node_next, inference);

                this->get_empirical_value(matrix_node_next->stats, outcome.value);
                // TODO use chance node? Breaks if matrix_node_next is terminal?
                this->update_matrix_stats(matrix_node->stats, outcome);
                this->update_chance_stats(chance_node->stats, outcome);
                return matrix_node_leaf;
            }
            else
            {
                if constexpr (MatrixNode::STORES_VALUE)
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
};