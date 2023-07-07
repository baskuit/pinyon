#pragma once

#include <libsurskit/math.hh>
#include <tree/node.hh>

#include <unordered_map>

template <typename Algorithm>
class ChanceNodeFlat;

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
        const size_t n_children = row_actions.size() * col_actions.size();
        edges = new ChanceNodeFlat<Algorithm> *[n_children];
        std::fill_n(edges, n_children, nullptr);
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
        const size_t n_children = row_actions.size() * col_actions.size();

        for (size_t i = 0; i < n_children; ++i)
        {
            ChanceNodeFlat<Algorithm> *&chance_node = edges[i];
            if (edges[i] != nullptr)
            {
                c += chance_node->count_matrix_nodes();
            }
        }
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

        for (const auto &[obs, matrix_node] : edges)
        {
            if (matrix_node != nullptr)
            {
                c += matrix_node->count_matrix_nodes();
            }
        }
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
    for (const auto &[obs, matrix_node] : edges)
    {
        delete matrix_node;
    }
};
