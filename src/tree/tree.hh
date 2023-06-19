#pragma once

#include <libsurskit/math.hh>
#include <tree/node.hh>

template <typename Algorithm>
class ChanceNode;

/*
Matrix Node
*/

template <typename Algorithm>
class MatrixNode : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    ChanceNode<Algorithm> *child = nullptr;
    MatrixNode<Algorithm> *next = nullptr;

    bool is_terminal = false;
    bool is_expanded = false;

    typename Types::VectorAction row_actions;
    typename Types::VectorAction col_actions;
    typename Types::Observation obs;
    typename Types::MatrixStats stats;

    MatrixNode(){};
    MatrixNode(
        typename Types::Observation obs) : obs(obs) {}
    ~MatrixNode();

    inline void expand(typename Types::State &state)
    {
        is_expanded = true;
        row_actions = state.row_actions;
        col_actions = state.col_actions;
        // state.get_actions (row_actions, col_actions); TODO
    }

    ChanceNode<Algorithm> *access(ActionIndex row_idx, int col_idx)
    {
        if (this->child == nullptr)
        {
            this->child = new ChanceNode<Algorithm>(row_idx, col_idx);
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
        ChanceNode<Algorithm> *child = new ChanceNode<Algorithm>(row_idx, col_idx);
        previous->next = child;
        return child;
    };

    size_t count_matrix_nodes()
    {
        size_t c = 1;
        ChanceNode<Algorithm> *current = this->child;
        while (current != nullptr)
        {
            c += current->count_matrix_nodes();
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

    MatrixNode<Algorithm> *child = nullptr;
    ChanceNode<Algorithm> *next = nullptr;

    ActionIndex row_idx;
    ActionIndex col_idx;

    typename Types::ChanceStats stats;

    ChanceNode () {}
    ChanceNode(
        ActionIndex row_idx,
        ActionIndex col_idx) : row_idx(row_idx), col_idx(col_idx) {}
    ~ChanceNode();

    MatrixNode<Algorithm> *access(typename Types::Observation &obs)
    {
        if (this->child == nullptr)
        {
            MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(obs);
            this->child = child;
            return child;
        }
        MatrixNode<Algorithm> *current = this->child;
        MatrixNode<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->obs == obs)
            {
                return current;
            }
            current = current->next;
        }
        MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(obs);
        previous->next = child;
        return child;
    };

    size_t count_matrix_nodes()
    {
        size_t c = 0;
        MatrixNode<Algorithm> *current = this->child;
        while (current != nullptr)
        {
            c += current->count_matrix_nodes();
            current = current->next;
        }
        return c;
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
};