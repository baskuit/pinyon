#pragma once

#include "../libsurskit/math.hh"

// TODO: Consider making these subclasses of "Node"
// template <typename Algorithm>
// class Node;

template <class _Algorithm>
class AbstractNode {
public:
    struct Types : _Algorithm::Types {
        using Algorithm = _Algorithm;
    };
};

template <typename Algorithm>
class ChanceNode;

// Matrix Node

template <typename Algorithm>
class MatrixNode : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };
    
    ChanceNode<Algorithm> *parent = nullptr;
    ChanceNode<Algorithm> *child = nullptr;
    MatrixNode<Algorithm> *prev = nullptr;
    MatrixNode<Algorithm> *next = nullptr;

    bool is_terminal = false;
    bool is_expanded = false;

    typename Types::Transition transition;
    typename Types::Actions actions;
    typename Types::Inference inference;
    typename Types::MatrixStats stats; // cumulative_value, vists now part of stats

    MatrixNode () {};
    MatrixNode(
        ChanceNode<Algorithm> *parent,
        MatrixNode<Algorithm> *prev,
        typename Types::Transition transition) : parent(parent), prev(prev), transition(transition) {}
    ~MatrixNode();

    ChanceNode<Algorithm> *access(int row_idx, int col_idx)
    {
        if (this->child == nullptr)
        {
            this->child = new ChanceNode<Algorithm>(this, nullptr, row_idx, col_idx);
            return this->child;
        }
        ChanceNode<Algorithm> *current = this->child;
        ChanceNode<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->row_idx == row_idx && current->col_idx == col_idx)
            {
                return current;
            }
            current = current->next;
        }
        ChanceNode<Algorithm> *child = new ChanceNode<Algorithm>(this, previous, row_idx, col_idx);
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
        ChanceNode<Algorithm> *current = this->child;
        while (current != nullptr)
        {
            c += current->count();
            current = current->next;
        }
        return c;
    }
};

// Chance Node
template <typename Algorithm>
class ChanceNode : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    MatrixNode<Algorithm> *parent = nullptr;
    MatrixNode<Algorithm> *child = nullptr;
    ChanceNode<Algorithm> *prev = nullptr;
    ChanceNode<Algorithm> *next = nullptr;

    int row_idx;
    int col_idx;

    typename Types::ChanceStats stats;

    ChanceNode(
        MatrixNode<Algorithm> *parent,
        ChanceNode<Algorithm> *prev,
        int row_idx,
        int col_idx) : parent(parent), prev(prev), row_idx(row_idx), col_idx(col_idx) {}
    ~ChanceNode();

    MatrixNode<Algorithm> *access(typename Types::Transition &transition)
    {
        if (this->child == nullptr)
        {
            MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(this, nullptr, transition);
            this->child = child;
            return child;
        }
        MatrixNode<Algorithm> *current = this->child;
        MatrixNode<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->transition.obs == transition.obs)
            {
                return current;
            }
            current = current->next;
        }
        MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(this, previous, transition);
        previous->next = child;
        return child;
    };

    int count()
    {
        int c = 0;
        MatrixNode<Algorithm> *current = this->child;
        while (current != nullptr)
        {
            c += current->count();
            current = current->next;
        }
        return c;
    }

    typename Types::Probability get_explored_total()
    {
        typename Types::Probability total(0, 1);
        MatrixNode<Algorithm> cur = child;
        while (cur != nullptr)
        {
            total += cur.transition.prob;
            cur = cur->next;
        }
        return total;
    }
};

// We have to hold off on destructor definitions until here

template <typename Algorithm>
MatrixNode<Algorithm>::~MatrixNode()
{

    while (this->child != nullptr)
    {
        ChanceNode<Algorithm> *victim = this->child;
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

template <typename Algorithm>
ChanceNode<Algorithm>::~ChanceNode()
{
    while (this->child != nullptr)
    {
        MatrixNode<Algorithm> *victim = this->child;
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