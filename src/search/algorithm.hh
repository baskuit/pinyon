#pragma once

template <class Algorith>
class ChanceNode;

template <class Algorithm>
class MatrixNode
{
public:
    bool is_terminal = false;
    bool is_expanded = false;
    ChanceNode<Algorithm> *access(int i, int j);
    typename Algorithm::Types::Actions actions;
    typename Algorithm::Types::Inference inference;
    typename Algorithm::Types::MatrixStats stats;
};

template <class Algorithm>
class ChanceNode
{
public:
    MatrixNode<Algorithm> *access(typename Algorithm::Types::Transition);
    typename Algorithm::Types::ChanceStats stats;
};

template <class _Model>
class AbstractAlgorithm
{
    static_assert(std::derived_from<_Model, DualPolicyValueModel<typename _Model::Types::State>> == true);

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

/*
This algorithm generalizes the typical "MCTS" pattern found in UCT, PUCT, SM-MCTS(-A), MatrixUCB etc.
This will also be subclassed for multithreaded implementations.*/

template <class Model>
class BanditAlgorithm : public AbstractAlgorithm<Model>
{
    static_assert(std::derived_from<Model, AbstractModel<typename Model::Types::State>> == true);

public:
    struct Outcome; // If you don't add to types, you have to typename BanditAlgorithm<Model> in the specification...
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = BanditAlgorithm::Outcome;
    };
    struct Outcome
    {
        int row_idx, col_idx;
        typename Types::Real row_value, col_value;
        typename Types::Real row_mu, col_mu;
        // Actor policy at row_idx, col_idx.
    };

    static void select(
        MatrixNode<BanditAlgorithm> *matrix_node,
        Outcome &outcome) {}
    static void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *matrix_node){};
    static void init_stats(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *matrix_node) {}
    static void update_matrix_node(MatrixNode<BanditAlgorithm> *matrix_node, Outcome &outcome){};
    static void update_chance_node(ChanceNode<BanditAlgorithm> *chance_node, Outcome &outcome){};
};

template <class Algorithm>
class TreeBandit : public Algorithm
{
    // static_assert(std::derived_from<Algorithm, Bandit<typename Algorithm::Types::Model>> == true);

public:
    // struct MatrixStats;
    struct Types : Algorithm::Types
    {
    };

    void run(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> &matrix_node)
    {
        Algorithm::init_stats(state, model, &matrix_node);
        for (int playout = 0; playout < playouts; ++playout)
        {
            _playout(state, model, &matrix_node);
        }
    }

private:
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

                Algorithm::select(matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = _playout(state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                Algorithm::update_matrix_node(matrix_node, outcome);
                Algorithm::update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                Algorithm::expand(state, model, matrix_node);
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }
};