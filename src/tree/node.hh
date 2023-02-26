#pragma once

#include "../libsurskit/math.hh"

// TODO: Consider making these subclasses of "Node"
// template <typename _Algorithm>
// class Node;

template <typename _Algorithm>
class ChanceNode;

// Matrix Node

template <typename _Algorithm>
class MatrixNode
{
public:
    using State = typename _Algorithm::State;
    using PlayerAction = typename _Algorithm::PlayerAction;
    using ChanceAction = typename _Algorithm::ChanceAction;
    using Number = typename _Algorithm::Number;
    using VectorDouble = typename _Algorithm::VectorDouble;
    using VectorInt = typename _Algorithm::VectorInt;
    using VectorAction = typename _Algorithm::VectorAction;
    using TransitionData = typename _Algorithm::TransitionData;
    using PairActions = typename _Algorithm::PairActions;
    using Model = typename _Algorithm::Model;
    using InferenceData = typename _Algorithm::InferenceData;
    using Algorithm = _Algorithm;
    using MatrixStats = typename _Algorithm::MatrixStats;
    using ChanceStats = typename _Algorithm::ChanceStats;

    ChanceNode<_Algorithm> *parent = nullptr;
    ChanceNode<_Algorithm> *child = nullptr;
    MatrixNode<_Algorithm> *prev = nullptr;
    MatrixNode<_Algorithm> *next = nullptr;

    bool is_terminal = false;
    bool is_expanded = false;

    TransitionData transition_data;
    PairActions pair_actions;
    InferenceData inference_data;
    MatrixStats stats; // cumulative_value, vists now part of stats

    MatrixNode () {};
    MatrixNode(
        ChanceNode<_Algorithm> *parent,
        MatrixNode<_Algorithm> *prev,
        TransitionData transition_data) : parent(parent), prev(prev), transition_data(transition_data) {}
    ~MatrixNode();

    ChanceNode<_Algorithm> *access(int row_idx, int col_idx)
    {
        if (this->child == nullptr)
        {
            this->child = new ChanceNode<_Algorithm>(this, nullptr, row_idx, col_idx);
            return this->child;
        }
        ChanceNode<_Algorithm> *current = this->child;
        ChanceNode<_Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->row_idx == row_idx && current->col_idx == col_idx)
            {
                return current;
            }
            current = current->next;
        }
        ChanceNode<_Algorithm> *child = new ChanceNode<_Algorithm>(this, previous, row_idx, col_idx);
        previous->next = child;
        return child;
    };

    // void make_terminal()
    // {
    //     while (child != nullptr)
    //     {
    //         delete child;
    //     }
    //     is_terminal = true;
    // }

    int count()
    {
        int c = 1;
        ChanceNode<_Algorithm> *current = this->child;
        while (current != nullptr)
        {
            c += current->count();
            current = current->next;
        }
        return c;
    }
};

// Chance Node

template <typename _Algorithm>
class ChanceNode
{
public:
    using State = typename _Algorithm::State;
    using PlayerAction = typename _Algorithm::PlayerAction;
    using ChanceAction = typename _Algorithm::ChanceAction;
    using Number = typename _Algorithm::Number;
    using VectorDouble = typename _Algorithm::VectorDouble;
    using VectorInt = typename _Algorithm::VectorInt;
    using VectorAction = typename _Algorithm::VectorAction;
    using TransitionData = typename _Algorithm::TransitionData;
    using PairActions = typename _Algorithm::PairActions;
    using Model = typename _Algorithm::Model;
    using InferenceData = typename _Algorithm::InferenceData;
    using Algorithm = _Algorithm;
    using MatrixStats = typename _Algorithm::MatrixStats;
    using ChanceStats = typename _Algorithm::ChanceStats;

    MatrixNode<_Algorithm> *parent = nullptr;
    MatrixNode<_Algorithm> *child = nullptr;
    ChanceNode<_Algorithm> *prev = nullptr;
    ChanceNode<_Algorithm> *next = nullptr;

    int row_idx;
    int col_idx;

    ChanceStats stats;

    ChanceNode<_Algorithm>(
        MatrixNode<_Algorithm> *parent,
        ChanceNode<_Algorithm> *prev,
        int row_idx,
        int col_idx) : parent(parent), prev(prev), row_idx(row_idx), col_idx(col_idx) {}
    ~ChanceNode<_Algorithm>();

    MatrixNode<_Algorithm> *access(TransitionData &transition_data)
    {
        if (this->child == nullptr)
        {
            MatrixNode<_Algorithm> *child = new MatrixNode<_Algorithm>(this, nullptr, transition_data);
            this->child = child;
            return child;
        }
        MatrixNode<_Algorithm> *current = this->child;
        MatrixNode<_Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->transition_data.chance_action == transition_data.chance_action)
            {
                return current;
            }
            current = current->next;
        }
        MatrixNode<_Algorithm> *child = new MatrixNode<_Algorithm>(this, previous, transition_data);
        previous->next = child;
        return child;
    };

    int count()
    {
        int c = 0;
        MatrixNode<_Algorithm> *current = this->child;
        while (current != nullptr)
        {
            c += current->count();
            current = current->next;
        }
        return c;
    }

    // Rational get_explored_total()
    // {
    //     Rational total(0, 1);
    //     MatrixNode<_Algorithm> cur = child;
    //     while (cur != nullptr)
    //     {
    //         total += cur.transition_data.probability;
    //         cur = cur->next;
    //     }
    //     return total;
    // }
};

// We have to hold off on destructor definitions until here

template <typename _Algorithm>
MatrixNode<_Algorithm>::~MatrixNode<_Algorithm>()
{

    while (this->child != nullptr)
    {
        ChanceNode<_Algorithm> *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
    if (this->prev != nullptr)
    {
        this->prev->next = this->next;
    }
    else if (this->parent != nullptr)
    {
        this->parent->child = this->next;
    }
}

template <typename _Algorithm>
ChanceNode<_Algorithm>::~ChanceNode<_Algorithm>()
{
    while (this->child != nullptr)
    {
        MatrixNode<_Algorithm> *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
    if (this->prev != nullptr)
    {
        this->prev->next = this->next;
    }
    else if (this->parent != nullptr)
    {
        this->parent->child = this->next;
    }
};