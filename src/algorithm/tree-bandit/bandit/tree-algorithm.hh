#pragma once

#include <types/types.hh>
#include <algorithm/algorithm.hh>

#include <chrono>

using namespace std::chrono;

template <class Model, template <class> class BanditAlgorithm, bool StopEarly = false>
class TreeBandit : public BanditAlgorithm<Model>
{
public:
    struct Types : BanditAlgorithm<Model>::Types
    {
    };

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> &matrix_node)
    {
        this->_initialize_stats(iterations, state, model, &matrix_node);
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
        MatrixNode<BanditAlgorithm> &matrix_node)
    {
        auto start = high_resolution_clock::now();
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);

        // this->_initialize_stats(iterations, state, model, &matrix_node);
        while (duration.count() < duration_us)
        {
            typename Types::State state_copy = state;
            state_copy.seed = device.template new_seed<uint64_t>();
            this->run_iteration(device, state_copy, model, &matrix_node);

            end = high_resolution_clock::now();
            duration = duration_cast<microseconds>(end - start);
        }
    }

protected:

    /*
    These functions below are overrided in the multithreaded implementations.
    */

    MatrixNode<BanditAlgorithm> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::ModelOutput &inference)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                Outcome outcome;

                select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<BanditAlgorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<BanditAlgorithm> *matrix_node_next = chance_node->access(state.obs);

                MatrixNode<BanditAlgorithm> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                update_matrix_node(matrix_node, outcome);
                update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                this->expand(state, model, matrix_node, inference);
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
        MatrixNode<BanditAlgorithm> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                Outcome outcome;

                _select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<BanditAlgorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<BanditAlgorithm> *matrix_node_next = chance_node->access(state.transition);

                run_iteration_average(device, state, model, matrix_node_next);

                _get_empirical_values(matrix_node_next, outcome.row_value, outcome.col_value);
                _update_matrix_node(matrix_node, outcome);
                _update_chance_node(chance_node, outcome);
                return;
            }
            else
            {
                this->_expand(state, model, matrix_node);
                return;
            }
        }
        else
        {
            return;
        }
    }
};