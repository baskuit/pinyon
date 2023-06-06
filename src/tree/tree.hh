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

    // ChanceNode<Algorithm> *parent = nullptr;
    ChanceNode<Algorithm> *child = nullptr;
    // MatrixNode<Algorithm> *prev = nullptr;
    // MatrixNode<Algorithm> *next = nullptr;

    bool is_terminal = false;
    bool is_expanded = false;

    typename Types::VectorAction row_actions;
    typename Types::VectorAction col_actions;
    // typename Types::Observation obs;
    typename Types::MatrixStats stats;

    MatrixNode(){};
    // MatrixNode(
        // typename Types::Observation obs) : obs(obs) {}
    // MatrixNode(
    //     ChanceNode<Algorithm> *parent,
    //     MatrixNode<Algorithm> *prev,
    //     typename Types::Observation obs) : parent(parent), prev(prev), obs(obs) {}
    ~MatrixNode();

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

    // void make_terminal()
    // {
    //     while (child != nullptr)
    //     {
    //         delete child;
    //     }
    //     is_terminal = true;
    // }

    size_t count_siblings()
    {
        // called on a matrix node to see how many branches its chance node parent has
        size_t c = 1;
        MatrixNode<Algorithm> *current = this->next;
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
        ChanceNode<Algorithm> *current = this->child;
        while (current != nullptr)
        {
            c += current->count_matrix_nodes();
            current = current->next;
        }
        return c;
    }

    // void spot_delete()
    // {
    //     if (this->prev != nullptr)
    //     {
    //         this->prev->next = this->next;
    //     }
    //     else if (this->parent != nullptr)
    //     {
    //         this->parent->child = this->next;
    //     }
    //     delete this;
    // }
};

// Chance Node
template <typename Algorithm>
class ChanceNode : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    struct L {

        MatrixNode<Algorithm>* matrix_node = nullptr;
        typename Types::Observation obs;
        L* next = nullptr;
        L () {}
        L (
            MatrixNode<Algorithm>* matrix_node,
            typename Types::Observation obs
        ) : matrix_node(matrix_node), obs(obs) {}
        ~L () {
            delete matrix_node;
            delete next;
        }
    };

    // MatrixNode<Algorithm> *parent = nullptr;
    // MatrixNode<Algorithm> *child = nullptr;
    // ChanceNode<Algorithm> *prev = nullptr;
    ChanceNode<Algorithm> *next = nullptr;
    L l;

    ActionIndex row_idx;
    ActionIndex col_idx;

    typename Types::ChanceStats stats;

    ChanceNode () {}
    ChanceNode(
        ActionIndex row_idx,
        ActionIndex col_idx) : row_idx(row_idx), col_idx(col_idx) {}
    // ChanceNode(
    //     MatrixNode<Algorithm> *parent,
    //     ChanceNode<Algorithm> *prev,
    //     ActionIndex row_idx,
    //     ActionIndex col_idx) : parent(parent), prev(prev), row_idx(row_idx), col_idx(col_idx) {}
    ~ChanceNode();

    MatrixNode<Algorithm> *access(typename Types::Observation &obs) // TODO check speed on pass-by
    {
        if (l.matrix_node == nullptr)
        {
            MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>();
            l.matrix_node = child;
            l.obs = obs;
            return child;
        }
        L *current = &l;
        L *previous = &l;
        while (current != nullptr)
        {
            previous = current;
            if (current->obs == obs)
            {
                return current->matrix_node;
            }
            current = current->next;
        }
        MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>();
        L* child_wrapper = new L(child, obs);
        previous->next = child_wrapper;
        return child;
    };

    size_t count_matrix_nodes()
    {
        size_t c = 0;
        auto current = &(this->l);
        while (current != nullptr)
        {
            c += current->matrix_node->count_matrix_nodes();
            current = current->next;
        }
        return c;
    }

    // size_t count_siblings()
    // {
    //     size_t c = 1;
    //     ChanceNode<Algorithm> *current = this->next;
    //     while (current != nullptr)
    //     {
    //         ++c;
    //         current = current->next;
    //     }
    //     current = this->prev;
    //     while (current != nullptr)
    //     {
    //         ++c;
    //         current = current->prev;
    //     }
    //     return c;
    // }

    // void spot_delete()
    // {
    //     if (this->prev != nullptr)
    //     {
    //         this->prev->next = this->next;
    //     }
    //     else if (this->parent != nullptr)
    //     {
    //         this->parent->child = this->next;
    //     }
    //     delete this;
    // }
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
    // delete &l
};

template <class Algorithm>
struct StackNode
{

    MatrixNode<Algorithm> root;
    std::vector<ChanceNode<Algorithm>> children;

    StackNode(
        typename Algorithm::Types::State &state)
    {
        state.get_actions();

        if (root.is_terminal = state.is_terminal)
        {
            // root.is_expanded = true;
            return;
        }

        root.row_actions = state.row_actions;
        root.col_actions = state.col_actions;

        size_t entries = state.row_actions.size() * state.col_actions.size();
        children.resize(entries);

        size_t index = 0;
        size_t row_idx = 0, col_idx = 0;
        for (const auto row_action : state.row_actions)
        {
            col_idx = 0;
            for (const auto col_action : state.col_actions)
            {
                auto state_copy = state;
                state_copy.apply_actions(row_action, col_action);
                ChanceNode<Algorithm> &child = children[index];
                child.parent = &root;
                if (index > 0)
                {
                    child.prev = &children[index - 1];
                }
                child.row_idx = row_idx;
                child.col_idx = col_idx;

                ++index;
                ++col_idx;
            }
            ++row_idx;
        }
    }
};
