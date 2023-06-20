#pragma once

#include <libsurskit/math.hh>
#include <tree/node.hh>

template <typename Algorithm>
class ChanceNodeDebug;

/*
Matrix Node
*/

template <typename Algorithm>
class MatrixNodeDebug : public AbstractNode<Algorithm>
{
public:

    static constexpr bool STORES_VALUE = false;

    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    ChanceNodeDebug<Algorithm> *parent = nullptr;
    ChanceNodeDebug<Algorithm> *child = nullptr;
    MatrixNodeDebug<Algorithm> *prev = nullptr;
    MatrixNodeDebug<Algorithm> *next = nullptr;

    bool terminal = false;
    bool expanded = false;

    typename Types::VectorAction row_actions;
    typename Types::VectorAction col_actions;
    typename Types::Observation obs;
    typename Types::MatrixStats stats;

    MatrixNodeDebug(){};
    MatrixNodeDebug(
        typename Types::Observation obs) : obs(obs) {}
    MatrixNodeDebug(
        ChanceNodeDebug<Algorithm> *parent,
        MatrixNodeDebug<Algorithm> *prev,
        typename Types::Observation obs) : parent(parent), prev(prev), obs(obs) {}
    ~MatrixNodeDebug();

    inline void expand(typename Types::State &state)
    {
        expanded = true;
        row_actions = state.row_actions;
        col_actions = state.col_actions;
    }

    void apply_actions(typename Types::State &state, const ActionIndex row_idx, const ActionIndex col_idx) const
    {
        state.apply_actions(row_actions[row_idx], col_actions[col_idx]);
    }

    typename Types::Action get_row_action(const ActionIndex row_idx) const
    {
        return row_actions[row_idx];
    }

    typename Types::Action get_col_action(const ActionIndex col_idx) const
    {
        return col_actions[col_idx];
    }

    inline bool is_terminal() const
    {
        return terminal;
    }

    inline bool is_expanded() const
    {
        return expanded;
    }

    inline void set_terminal()
    {
        terminal = true;
    }

    inline void set_expanded()
    {
        expanded = true;
    }

    ChanceNodeDebug<Algorithm> *access(ActionIndex row_idx, int col_idx)
    {
        if (this->child == nullptr)
        {
            this->child = new ChanceNodeDebug<Algorithm>(row_idx, col_idx);
            return this->child;
        }
        ChanceNodeDebug<Algorithm> *current = this->child;
        ChanceNodeDebug<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->row_idx == row_idx && current->col_idx == col_idx)
            {
                return current;
            }
            current = current->next;
        }
        ChanceNodeDebug<Algorithm> *child = new ChanceNodeDebug<Algorithm>(row_idx, col_idx);
        previous->next = child;
        return child;
    };

    size_t count_siblings()
    {
        // called on a matrix node to see how many branches its chance node parent has
        size_t c = 1;
        MatrixNodeDebug<Algorithm> *current = this->next;
        while (current != nullptr)
        {
            ++c;
            current = current->next;
        }
        current = this->prev;
        while (current != nullptr)
        {
            ++c;
            current = current->prev;
        }
        return c;
    }

    size_t count_matrix_nodes()
    {
        size_t c = 1;
        ChanceNodeDebug<Algorithm> *current = this->child;
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
class ChanceNodeDebug : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    MatrixNodeDebug<Algorithm> *parent = nullptr;
    MatrixNodeDebug<Algorithm> *child = nullptr;
    ChanceNodeDebug<Algorithm> *prev = nullptr;
    ChanceNodeDebug<Algorithm> *next = nullptr;

    ActionIndex row_idx;
    ActionIndex col_idx;

    typename Types::ChanceStats stats;

    ChanceNodeDebug() {}
    ChanceNodeDebug(
        ActionIndex row_idx,
        ActionIndex col_idx) : row_idx(row_idx), col_idx(col_idx) {}
    ChanceNodeDebug(
        MatrixNodeDebug<Algorithm> *parent,
        ChanceNodeDebug<Algorithm> *prev,
        ActionIndex row_idx,
        ActionIndex col_idx) : parent(parent), prev(prev), row_idx(row_idx), col_idx(col_idx) {}
    ~ChanceNodeDebug();

    MatrixNodeDebug<Algorithm> *access(typename Types::Observation &obs)
    {
        if (this->child == nullptr)
        {
            MatrixNodeDebug<Algorithm> *child = new MatrixNodeDebug<Algorithm>(this, nullptr, obs);
            this->child = child;
            return child;
        }
        MatrixNodeDebug<Algorithm> *current = this->child;
        MatrixNodeDebug<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->obs == obs)
            {
                return current;
            }
            current = current->next;
        }
        MatrixNodeDebug<Algorithm> *child = new MatrixNodeDebug<Algorithm>(this, previous, obs);
        previous->next = child;
        return child;
    };

    size_t count_matrix_nodes()
    {
        size_t c = 0;
        MatrixNodeDebug<Algorithm> *current = this->child;
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
MatrixNodeDebug<Algorithm>::~MatrixNodeDebug()
{
    while (this->child != nullptr)
    {
        ChanceNodeDebug<Algorithm> *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
}

template <typename Algorithm>
ChanceNodeDebug<Algorithm>::~ChanceNodeDebug()
{
    while (this->child != nullptr)
    {
        MatrixNodeDebug<Algorithm> *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
};