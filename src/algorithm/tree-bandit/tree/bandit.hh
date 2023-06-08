#pragma once

#include <types/types.hh>
// #include <tree/tree.hh>
#include <algorithm/algorithm.hh>

#include <thread>
#include <chrono>

using namespace std::chrono;

// outcome struct where we only store the policy for the selected action (indices) for either player

template <typename Model>
struct ChoicesOutcome
{
    using Real = typename Model::Types::Real;
    ActionIndex row_idx, col_idx;
    typename Model::Types::Value value;
    Real row_mu, col_mu;
};

// outcome struct where we store the entire policy, for all actions

template <typename Model>
struct PolicyOutcome
{
    using Real = typename Model::Types::Real;
    using VectorReal = typename Model::Types::VectorReal;

    ActionIndex row_idx, col_idx;
    typename Model::Types::Value value;
    VectorReal row_policy, col_policy;
};

template <class Model, class BanditAlgorithm, 
    template <class M> class _Outcome, template <class A> class MatrixNode, template <class A> class ChanceNode>
class TreeBandit : public AbstractAlgorithm<Model>
{
public:
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = _Outcome<Model>;
    };

    void run(
        size_t iterations,
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
            state_copy.reseed(device.template new_seed<uint64_t>());
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
        const typename Types::State &state,
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
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::ModelOutput &inference)
    {
        state.get_actions();
        matrix_node->row_actions = state.row_actions;
        matrix_node->col_actions = state.col_actions;
        matrix_node->is_expanded = true;
        matrix_node->is_terminal = state.is_terminal;

        static_cast<BanditAlgorithm *>(this)->expand(
            state,
            model,
            matrix_node);

        // have to do this last, since monte carlo model does not clone, so it rolls the state out during inference.

        if (matrix_node->is_terminal)
        {
            inference.value = state.payoff;
        }
        else
        {
            model.get_inference(state, inference);
        }
    }

    void _update_matrix_node(
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::Outcome &outcome)
    {
        return static_cast<BanditAlgorithm *>(this)->update_matrix_node(
            matrix_node,
            outcome);
    }

    void _update_chance_node(
        ChanceNode<BanditAlgorithm> *chance_node,
        typename Types::Outcome &outcome)
    {
        return static_cast<BanditAlgorithm *>(this)->update_chance_node(
            chance_node,
            outcome);
    }

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
                typename Types::Outcome outcome;

                _select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<BanditAlgorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<BanditAlgorithm> *matrix_node_next = chance_node->access(state.obs);

                MatrixNode<BanditAlgorithm> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                _update_matrix_node(matrix_node, outcome);
                _update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                this->_expand(state, model, matrix_node, inference);
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
                typename Types::Outcome outcome;

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