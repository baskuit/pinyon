#pragma once

#include "tree/tree.hh"
#include "algorithm/algorithm.hh"

#include <thread>
#include <chrono>

template <class Model, class Algorithm>
class TreeBanditBase : public AbstractAlgorithm<Model>
{
public:
    // Would have to pass Outcome type as template parameter if you wanted to specialize it. Have to pass Model since Algorithm is incomplete.
    struct Outcome;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = TreeBanditBase::Outcome;
    };

    struct Outcome
    {
        int row_idx, col_idx;
        typename Types::Real row_value, col_value;
        typename Types::Real row_mu, col_mu; // TODO should this be Float?
    };

    MatrixNode<Algorithm> root;

    void run(
        int iterations,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model)
    {
        run(iterations, device, state, model, root);
    }

    void run(
        int playouts,
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> &matrix_node)
    {
        this->_initialize_stats(playouts, state, model, &matrix_node);
        for (int playout = 0; playout < playouts; ++playout)
        {
            typename Types::State state_copy = state;
            this->_playout(device, state_copy, model, &matrix_node);
        }
    }

protected:
    void _get_empirical_strategies(
        MatrixNode<Algorithm> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        return static_cast<Algorithm *>(this)->get_empirical_strategies(
            matrix_node,
            row_strategy,
            col_strategy);
    }

    void _get_empirical_values(
        MatrixNode<Algorithm> *matrix_node,
        typename Types::Real &row_value,
        typename Types::Real &col_value)
    {
        return static_cast<Algorithm *>(this)->get_empirical_values(
            matrix_node,
            row_value,
            col_value);
    }

    MatrixNode<Algorithm> *_playout(
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        return static_cast<Algorithm *>(this)->playout(
            device,
            state,
            model,
            matrix_node);
    }
    void _select(
        prng &device,
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        return static_cast<Algorithm *>(this)->select(
            device,
            matrix_node,
            outcome);
    }
    void _initialize_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *root)
    {
        return static_cast<Algorithm *>(this)->initialize_stats(
            playouts,
            state,
            model,
            root);
    }
    void _expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        return static_cast<Algorithm *>(this)->expand(
            state,
            model,
            matrix_node); // TODO consider adding inference code here. It's all the same
    }
    void _update_matrix_node(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        return static_cast<Algorithm *>(this)->update_matrix_node(
            matrix_node,
            outcome);
    }

    void _update_chance_node(
        ChanceNode<Algorithm> *chance_node,
        Outcome &outcome)
    {
        return static_cast<Algorithm *>(this)->update_chance_node(
            chance_node,
            outcome);
    }
};

/*
Tree Bandit (single threaded)
*/

template <class Model, class Algorithm>
class TreeBandit : public TreeBanditBase<Model, Algorithm>
{
public:
    struct Types : TreeBanditBase<Model, Algorithm>::Types
    {
    };

    MatrixNode<Algorithm> *run_iteration(
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                this->_select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->run_iteration(device, state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                this->_update_matrix_node(matrix_node, outcome);
                this->_update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                this->_expand(state, model, matrix_node);
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }

    // TODO rename. MCTS-A style search where you return the empirical average values of the next node instead of the leaf node value.

    MatrixNode<Algorithm> *run_iteration_average(
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                this->_select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->run_iteration_average(device, state, model, matrix_node_next);

                this->_get_empirical_values(matrix_node_next, outcome.row_value, outcome.col_value);
                this->_update_matrix_node(matrix_node, outcome);
                this->_update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                this->_expand(state, model, matrix_node);
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }
};