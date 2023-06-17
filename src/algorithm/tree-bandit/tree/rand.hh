#pragma once

#include <libsurskit/math.hh>

/*
Minimal model for benchmarking purposes (Test speed of state and tree structure)
*/

template <
    class Model, 
    template <class _Model, class _BanditAlgorithm, template <class M> class Outcome, template <class A> class MNode, template <class A> class CNode> 
        class _TreeBandit = TreeBandit,
    template <class Algo> class _MatrixNode = MatrixNode,
    template <class Algo> class _ChanceNode = ChanceNode
>
class Rand : public _TreeBandit<Model, Exp3<Model, _TreeBandit>, ChoicesOutcome, _MatrixNode, _ChanceNode>
{

public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : _TreeBandit<Model, Rand<Model, _TreeBandit>, ChoicesOutcome, _MatrixNode, _ChanceNode>::Types
    {
        using MatrixStats = Rand::MatrixStats;
        using ChanceStats = Rand::ChanceStats;
    };
    struct MatrixStats : _TreeBandit<Model, Rand<Model, _TreeBandit>, ChoicesOutcome, _MatrixNode, _ChanceNode>::MatrixStats
    {
    };

    struct ChanceStats : _TreeBandit<Model, Rand<Model, _TreeBandit>, ChoicesOutcome, _MatrixNode, _ChanceNode>::ChanceStats
    {
    };

    Rand() {}

    friend std::ostream &operator<<(std::ostream &os, const Rand &session)
    {
        os << "Rand";
        return os;
    }

    void get_empirical_strategies(
        MatrixNode<Rand> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
    }

    void get_empirical_values(
        MatrixNode<Rand> *matrix_node,
        typename Types::Real &row_value,
        typename Types::Real &col_value)
    {
    }

    void initialize_stats(
        int iterations,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Rand> *matrix_node)
    {
    }

    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Rand> *matrix_node)
    {
    }

    void select(
        typename Types::PRNG &device,
        MatrixNode<Rand> *matrix_node,
        typename Types::Outcome &outcome)
    {
        const int rows = matrix_node->row_actions.size();
        const int cols = matrix_node->col_actions.size();
        const int row_idx = device.random_int(rows);
        const int col_idx = device.random_int(cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
    }

    void update_matrix_node(
        MatrixNode<Rand> *matrix_node,
        typename Types::Outcome &outcome)
    {
    }

    void update_chance_node(
        ChanceNode<Rand> *chance_node,
        typename Types::Outcome &outcome)
    {
    }
};