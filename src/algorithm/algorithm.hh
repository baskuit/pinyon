#pragma once

#include "../tree/tree.hh"

#include <thread>
#include <chrono>

template <class _Model>
class AbstractAlgorithm
{
    static_assert(std::derived_from<_Model, AbstractModel<typename _Model::Types::State>> == true);

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
    static_assert(std::derived_from<Model, DoubleOracleModel<typename Model::Types::State>> == true,
                  "Model parameter for TreeBanditBase must provide a value and policy estimate; Both Exp3p and MatrixUCB have time parameters that need policy to be estimated on expansion");
    // static_assert(std::derived_from<Algorithm, TreeBanditBase<Model, Algorithm>>,
    // "Algorithm parameter for TreeBanditBase must derive from TreeBanditBase, i.e. it must be a bandit algorithm implementation");
    // The above is not possible since Algorithm is not complete yet.
public:
    // Would have to pass Outcome type as template parameter if you wanted to specialize it.
    // Have to pass Model since Algorithm is incomplete.
    struct Outcome;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = TreeBanditBase::Outcome;
    };

    struct Outcome
    {
        int row_idx, col_idx;
        typename Types::Real row_value, col_value;
        typename Types::Real row_mu, col_mu;
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
            typename Types::State state_ = state;
            this->playout(state_, model, &matrix_node);
        }
    }
    void get_strategies(
        MatrixNode<Algorithm> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        return static_cast<Algorithm *>(this)->_get_strategies(
            matrix_node,
            row_strategy,
            col_strategy);
    }
    void run_for_duration(
        double duration_ms,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> &matrix_node)
    {
        const int playout_estimate = duration_ms * 1000;
        init_stats(playout_estimate, state, model, &matrix_node);
        int playouts = 0;
        for (
            auto start_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < duration_ms;
            ++playouts)
        {
            typename Types::State state_ = state;
            this->playout(state_, model, &matrix_node);
        }
        std::cout << "total playouts for duration: " << duration_ms << " ms: " << playouts << std::endl;
        std::cout << "playout estimate was : " << playout_estimate << std::endl;
    }

    MatrixNode<Algorithm> *playout(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        return static_cast<Algorithm *>(this)->_playout(
            state,
            model,
            matrix_node);
    }
    void select(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        return static_cast<Algorithm *>(this)->_select(
            matrix_node,
            outcome);
    }
    void init_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *root)
    {
        return static_cast<Algorithm *>(this)->_init_stats(
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
        return static_cast<Algorithm *>(this)->_expand(
            state,
            model,
            matrix_node);
    }
    void update_matrix_node(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        return static_cast<Algorithm *>(this)->_update_matrix_node(
            matrix_node,
            outcome);
    }

    void update_chance_node(
        ChanceNode<Algorithm> *chance_node,
        Outcome &outcome)
    {
        return static_cast<Algorithm *>(this)->_update_chance_node(
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