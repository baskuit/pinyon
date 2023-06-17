#pragma once

#include <types/types.hh>
#include <algorithm/algorithm.hh>

#include <chrono>

using namespace std::chrono;

template <class Model, template <class> class BanditAlgorithm, bool StopEarly = false>
class TreeBandit : public BanditAlgorithm<Model>
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : BanditAlgorithm<Model>::Types
    {
        using MatrixStats = TreeBandit::MatrixStats;
        using ChanceStats = TreeBandit::ChanceStats;
    };

    struct MatrixStats : BanditAlgorithm<Model>::MatrixStats
    {
    };
    struct ChanceStats : BanditAlgorithm<Model>::ChanceStats
    {
    };

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> &matrix_node)
    {
        this->initialize_stats(iterations, state, model, matrix_node.stats);
        typename Types::ModelOutput inference;
        for (int iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device.template new_seed<typename Types::Seed>());
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
        auto start = high_resolution_clock::now();
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);

        this->initialize_stats(iterations, state, model, matrix_node.stats);
        while (duration.count() < duration_us)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device.template new_seed<typename Types::Seed>());
            this->run_iteration(device, state_copy, model, &matrix_node);

            end = high_resolution_clock::now();
            duration = duration_cast<microseconds>(end - start);
        }
    }

protected:
    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> *matrix_node,
        typename Types::ModelOutput &inference)
    {
        state.get_actions();
        matrix_node->row_actions = state.row_actions;
        matrix_node->col_actions = state.col_actions;
        matrix_node->is_expanded = true;
        matrix_node->is_terminal = state.is_terminal;

        expand(state, model, matrix_node->stats);

        if (matrix_node->is_terminal)
        {
            inference.value = state.payoff;
        }
        else
        {
            model.get_inference(state, inference);
        }
    }

    MatrixNode<TreeBandit> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> *matrix_node,
        typename Types::ModelOutput &inference)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                Outcome outcome;

                select(device, matrix_node->stats, outcome);

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<TreeBandit> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<TreeBandit> *matrix_node_next = chance_node->access(state.obs);

                MatrixNode<BanditAlTreeBanditgorithm> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                update_matrix_node(matrix_node->stats, outcome);
                update_chance_node(chance_node.stats, outcome);
                return matrix_node_leaf;
            }
            else
            {
                this->expand(state, model, matrix_node->stats, inference);
                return matrix_node;
            }
        }
        else
        {
            inference.value = state.payoff;
            return matrix_node;
        }
    }

    // TODO rename. MCTS-A style search where you return the empirical average values of the next node instead of the leaf node value.

    void run_iteration_average(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBandit> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                Outcome outcome;

                select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<TreeBandit> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<TreeBandit> *matrix_node_next = chance_node->access(state.transition);

                run_iteration_average(device, state, model, matrix_node_next);

                get_empirical_values(matrix_node_next, outcome.row_value, outcome.col_value);
                update_matrix_node(matrix_node, outcome);
                update_chance_node(chance_node, outcome);
                return;
            }
            else
            {
                this->expand(state, model, matrix_node);
                return;
            }
        }
        else
        {
            return;
        }
    }
};