#pragma once

#include <libsurskit/math.hh>
#include <tree/node.hh>

template <typename Algorithm>
class MatrixNodeL;

template <typename Algorithm>
class ChanceNodeL;

struct LNodes
{
    template <typename Algorithm>
    using MNode = MatrixNodeL<Algorithm>;
    template <typename Algorithm>
    using CNode = ChanceNodeL<Algorithm>;
};

/*
Matrix Node
*/

template <typename Algorithm>
class MatrixNodeL : public AbstractNode<Algorithm>
{
public:
    static constexpr bool STORES_VALUE = false;

    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    ChanceNodeL<Algorithm> *child = nullptr;

    bool terminal = false;
    bool expanded = false;

    typename Types::VectorAction row_actions;
    typename Types::VectorAction col_actions;
    typename Types::MatrixStats stats;

    MatrixNodeL(){};
    ~MatrixNodeL();

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

    ChanceNodeL<Algorithm> *access(ActionIndex row_idx, int col_idx)
    {
        if (this->child == nullptr)
        {
            this->child = new ChanceNodeL<Algorithm>(row_idx, col_idx);
            return this->child;
        }
        ChanceNodeL<Algorithm> *current = this->child;
        ChanceNodeL<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->row_idx == row_idx && current->col_idx == col_idx)
            {
                return current;
            }
            current = current->next;
        }
        ChanceNodeL<Algorithm> *child = new ChanceNodeL<Algorithm>(row_idx, col_idx);
        previous->next = child;
        return child;
    };

    size_t count_siblings()
    {
        // called on a matrix node to see how many branches its chance node parent has
        size_t c = 1;
        MatrixNodeL<Algorithm> *current = this->next;
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
        ChanceNodeL<Algorithm> *current = this->child;
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
class ChanceNodeL : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    struct Edges
    {

        MatrixNodeL<Algorithm> *matrix_node = nullptr;
        typename Types::Observation obs;
        Edges *next = nullptr;
        Edges() {}
        Edges(
            MatrixNodeL<Algorithm> *matrix_node,
            typename Types::Observation obs) : matrix_node(matrix_node), obs(obs) {}
        ~Edges()
        {
            delete matrix_node;
            delete next;
        }
    };

    ChanceNodeL<Algorithm> *next = nullptr;
    Edges edge;

    ActionIndex row_idx;
    ActionIndex col_idx;

    typename Types::ChanceStats stats;

    ChanceNodeL() {}
    ChanceNodeL(
        ActionIndex row_idx,
        ActionIndex col_idx) : row_idx(row_idx), col_idx(col_idx) {}
    ~ChanceNodeL();

    MatrixNodeL<Algorithm> *access(typename Types::Observation &obs) // TODO check speed on pass-by
    {
        if (edge.matrix_node == nullptr)
        {
            MatrixNodeL<Algorithm> *child = new MatrixNodeL<Algorithm>();
            edge.matrix_node = child;
            edge.obs = obs;
            return child;
        }
        Edges *current = &edge;
        Edges *previous = &edge;
        while (current != nullptr)
        {
            previous = current;
            if (current->obs == obs)
            {
                return current->matrix_node;
            }
            current = current->next;
        }
        MatrixNodeL<Algorithm> *child = new MatrixNodeL<Algorithm>();
        Edges *child_wrapper = new Edges(child, obs);
        previous->next = child_wrapper;
        return child;
    };

    size_t count_matrix_nodes()
    {
        size_t c = 0;
        auto current = &(this->edge);
        while (current != nullptr)
        {
            c += current->matrix_node->count_matrix_nodes();
            current = current->next;
        }
        return c;
    }
};

// We have to hold off on destructor definitions until here

template <typename Algorithm>
MatrixNodeL<Algorithm>::~MatrixNodeL()
{
    while (this->child != nullptr)
    {
        ChanceNodeL<Algorithm> *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
}

template <typename Algorithm>
ChanceNodeL<Algorithm>::~ChanceNodeL(){
    // delete &l TODO lamo
};
