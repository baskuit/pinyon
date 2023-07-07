#pragma once

#include <libsurskit/math.hh>
#include <tree/node.hh>

#include <unordered_map>

template <typename Algorithm>
class MatrixNodeFlat;

template <typename Algorithm>
class ChanceNodeFlat;

struct FlatNodes
{
    template <typename Algorithm>
    using MNode = MatrixNodeFlat<Algorithm>;
    template <typename Algorithm>
    using CNode = ChanceNodeFlat<Algorithm>;
};

/*
Matrix Node
*/

template <typename Algorithm>
class MatrixNodeFlat : public AbstractNode<Algorithm>
{
public:
    static constexpr bool STORES_VALUE = false;

    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    bool terminal = false;
    bool expanded = false;
    typename Types::Observation obs;

    typename Types::VectorAction row_actions;
    typename Types::VectorAction col_actions;
    typename Types::MatrixStats stats;

    ChanceNodeFlat<Algorithm> **edges;

    MatrixNodeFlat(){};
    MatrixNodeFlat(
        typename Types::Observation obs) : obs(obs) {}
    ~MatrixNodeFlat();

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

    ChanceNodeFlat<Algorithm> *access(ActionIndex row_idx, int col_idx)
    {
        ActionIndex child_idx = row_idx * col_actions.size() + col_idx;
        ChanceNodeFlat<Algorithm> *&child = edges[child_idx];
        if (child == nullptr)
        {
            child = new ChanceNodeFlat<Algorithm>();
        }
        return child;
    };

    size_t count_matrix_nodes()
    {
        size_t c = 1;
        // ChanceNodeFlat<Algorithm> *current = this->child;
        // while (current != nullptr)
        // {
        //     c += current->count_matrix_nodes();
        //     current = current->next;
        // }
        return c;
    }
};

// Chance Node
template <typename Algorithm>
class ChanceNodeFlat : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    std::unordered_map<typename Types::Observation, MatrixNodeFlat<Algorithm> *, typename Types::ObservationHash> edges{};
    typename Types::ChanceStats stats{};

    ChanceNodeFlat() {}
    ~ChanceNodeFlat();

    MatrixNodeFlat<Algorithm> *access(typename Types::Observation &obs)
    {
        MatrixNodeFlat<Algorithm> *&child = edges[obs];
        if (child == nullptr)
        {
            child = new MatrixNodeFlat<Algorithm>(obs);
            return child;
        }
        return child;
    };

    size_t count_matrix_nodes()
    {
        size_t c = 0;
        // MatrixNodeFlat<Algorithm> *current = this->child;
        // while (current != nullptr)
        // {
        //     c += current->count_matrix_nodes();
        //     current = current->next;
        // }
        return c;
    }
};

// We have to hold off on destructor definitions until here

template <typename Algorithm>
MatrixNodeFlat<Algorithm>::~MatrixNodeFlat()
{
    delete[] edges;
}

template <typename Algorithm>
ChanceNodeFlat<Algorithm>::~ChanceNodeFlat()
{
    while (this->child != nullptr)
    {
        MatrixNodeFlat<Algorithm> *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
};
