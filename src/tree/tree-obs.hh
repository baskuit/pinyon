#pragma once

#include <libsurskit/math.hh>
#include <state/state.hh>
#include <tree/node.hh>

template <IsStateTypes Types, typename MatrixStats, typename ChanceStats>
struct LNodes : Types
{
    /*
    Instead of MatrixNodes storing their obs memeber, they are bundled with 'Edge' struct
    which is basically a linked list that the chance node parent owns
    */

    class MatrixNode;

    class ChanceNode;

    class MatrixNode
    {
    public:
        static constexpr bool STORES_VALUE = false;

        ChanceNode *child = nullptr;

        bool terminal = false;
        bool expanded = false;

        typename Types::VectorAction row_actions;
        typename Types::VectorAction col_actions;
        typename Types::MatrixStats stats;

        MatrixNode(){};
        ~MatrixNode();

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

        inline void get_value(typename Types::Value &value)
        {
        }

        ChanceNode *access(ActionIndex row_idx, int col_idx)
        {
            if (this->child == nullptr)
            {
                this->child = new ChanceNode(row_idx, col_idx);
                return this->child;
            }
            ChanceNode *current = this->child;
            ChanceNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->row_idx == row_idx && current->col_idx == col_idx)
                {
                    return current;
                }
                current = current->next;
            }
            ChanceNode *child = new ChanceNode(row_idx, col_idx);
            previous->next = child;
            return child;
        };

        size_t count_siblings()
        {
            // called on a matrix node to see how many branches its chance node parent has
            size_t c = 1;
            MatrixNode *current = this->next;
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
            ChanceNode *current = this->child;
            while (current != nullptr)
            {
                c += current->count_matrix_nodes();
                current = current->next;
            }
            return c;
        }
    };

    class ChanceNode
    {
    public:
        struct Edge
        {
            MatrixNode *matrix_node = nullptr;
            typename Types::Obs obs;
            Edge *next = nullptr;
            Edge() {}
            Edge(
                MatrixNode *matrix_node,
                typename Types::Obs obs) : matrix_node(matrix_node), obs(obs) {}
            ~Edge()
            {
                delete matrix_node;
                delete next;
            }
        };

        ChanceNode *next = nullptr;
        Edge edge;

        ActionIndex row_idx;
        ActionIndex col_idx;

        typename Types::ChanceStats stats;

        ChanceNode() {}
        ChanceNode(
            ActionIndex row_idx,
            ActionIndex col_idx) : row_idx(row_idx), col_idx(col_idx) {}
        ~ChanceNode();

        MatrixNode *access(typename Types::Obs &obs) // TODO check speed on pass-by
        {
            if (edge.matrix_node == nullptr)
            {
                MatrixNode *child = new MatrixNode();
                edge.matrix_node = child;
                edge.obs = obs;
                return child;
            }
            Edge *current = &edge;
            Edge *previous = &edge;
            while (current != nullptr)
            {
                previous = current;
                if (current->obs == obs)
                {
                    return current->matrix_node;
                }
                current = current->next;
            }
            MatrixNode *child = new MatrixNode();
            Edge *child_wrapper = new Edge(child, obs);
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
};

template <IsStateTypes Types, typename MatrixStats, typename ChanceStats>
LNodes<Types, MatrixStats, ChanceStats>::MatrixNode::~MatrixNode()
{
    while (this->child != nullptr)
    {
        LNodes<Types, MatrixStats, ChanceStats>::ChanceNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
}
template <IsStateTypes Types, typename MatrixStats, typename ChanceStats>
LNodes<Types, MatrixStats, ChanceStats>::ChanceNode::~ChanceNode()
{
    while (this->child != nullptr)
    {
        LNodes<Types, MatrixStats, ChanceStats>::MatrixNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
};