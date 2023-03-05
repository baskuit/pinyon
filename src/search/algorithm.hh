#pragma once

#include "../tree/node.hh"

#include <thread>
#include <mutex>

template <class _Model>
class AbstractAlgorithm
{
    // static_assert(std::derived_from<_Model, DualPolicyValueModel<typename _Model::Types::State>>,
    // "Even AbstractAlgorithm currently requires a DualPolicyValueModel");

public:
    struct MatrixStats
    {
    };
    struct ChanceStats
    {
    };
    struct Types : _Model::Types
    {
        using Model = _Model;
        using MatrixStats = AbstractAlgorithm::MatrixStats;
        using ChanceStats = AbstractAlgorithm::ChanceStats;
    };
};

template <class Model, class Algorithm>
class TreeBanditBase : public AbstractAlgorithm<Model>
{
public:
    struct Outcome;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = TreeBanditBase::Outcome;
    };

    struct Outcome
    {
        int row_idx, col_idx;
        typename Types::Real row_value, col_value;
        typename Types::Real row_mu, col_mu; // Actor policy at row_idx, col_idx.
    };

    void run(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> &matrix_node)
    {
        init_stats(playouts, state, model, &matrix_node);
        for (int playout = 0; playout < playouts; ++playout)
        {
            this->playout(state, model, &matrix_node);
        }
    }

    MatrixNode<Algorithm> *playout(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        static_cast<Algorithm *>(this)->_playout(
            state,
            model,
            matrix_node);
    }

    void select(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        static_cast<Algorithm *>(this)->_select(
            matrix_node,
            outcome);
    }

    void init_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *root)
    {
        static_cast<Algorithm *>(this)->_init_stats(
            playouts,
            state,
            model,
            root);
    }
    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        static_cast<Algorithm *>(this)->_expand(
            state,
            model,
            matrix_node);
    }
    void update_matrix_node(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        static_cast<Algorithm *>(this)->_update_matrix_node(
            matrix_node,
            outcome);
    }

    void update_chance_node(
        ChanceNode<Algorithm> *chance_node,
        Outcome &outcome)
    {
        static_cast<Algorithm *>(this)->_update_chance_node(
            chance_node,
            outcome);
    }
};

template <class Model, class Algorithm>
class TreeBandit : public TreeBanditBase<Model, Algorithm>
{
public:
    struct Types : TreeBanditBase<Model, Algorithm>::Types
    {
    };

    MatrixNode<Algorithm> *_playout(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                this->select(matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->_playout(state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                this->update_matrix_node(matrix_node, outcome);
                this->update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                this->expand(state, model, matrix_node);
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }
};

/*
TreeBandit with mutex's in each matrix node
*/

template <class Model, class Algorithm>
class TreeBanditThreaded : public TreeBanditBase<Model, Algorithm>
{
public:
    struct MatrixStats;
    struct Types : TreeBanditBase<Model, Algorithm>::Types
    {
        using MatrixStats = TreeBanditThreaded::MatrixStats;
    };

    struct MatrixStats
    {
        std::mutex mtx;
    };

    MatrixNode<Algorithm> *_playout(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {

        std::mutex &mtx = matrix_node->stats.mtx;

        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                mtx.lock();
                this->select(matrix_node, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->_playout(state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                mtx.lock();
                this->update_matrix_node(matrix_node, outcome);
                this->update_chance_node(chance_node, outcome);
                mtx.unlock();
                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                this->expand(state, model, matrix_node);
                mtx.unlock();
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }
};