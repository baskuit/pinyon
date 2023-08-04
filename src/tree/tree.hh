#pragma once

#include <libsurskit/math.hh>
#include <state/state.hh>
#include <tree/node.hh>

template <IsStateTypes Types, typename MatrixStats, typename ChanceStats>
struct DefaultNodes : Types
{
    class MatrixNode;

    class ChanceNode;

    class MatrixNode
    {
    public:
        static constexpr bool STORES_VALUE = false;

        ChanceNode *child = nullptr;
        MatrixNode *next = nullptr;

        bool terminal = false;
        bool expanded = false;

        typename Types::VectorAction row_actions;
        typename Types::VectorAction col_actions;
        typename Types::Obs obs;
        MatrixStats stats;



        MatrixNode(){};
        MatrixNode(typename Types::Obs obs) : obs(obs) {}
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

        typename Types::VectorAction get_row_actions () const
        {
            return row_actions;
        }

        typename Types::VectorAction get_col_actions () const
        {
            return col_actions;
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

        inline void set_terminal(const bool value)
        {
            terminal = value;
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

    // Chance Node
    class ChanceNode
    {
    public:
        MatrixNode *child = nullptr;
        ChanceNode *next = nullptr;

        ActionIndex row_idx;
        ActionIndex col_idx;

        ChanceStats stats;

        ChanceNode() {}
        ChanceNode(
            ActionIndex row_idx,
            ActionIndex col_idx) : row_idx(row_idx), col_idx(col_idx) {}
        ~ChanceNode();

        MatrixNode *access(const typename Types::Obs &obs)
        {
            if (this->child == nullptr)
            {
                MatrixNode *child = new MatrixNode(obs);
                this->child = child;
                return child;
            }
            MatrixNode *current = this->child;
            MatrixNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->obs == obs)
                {
                    return current;
                }
                current = current->next;
            }
            MatrixNode *child = new MatrixNode(obs);
            previous->next = child;
            return child;
        };

        size_t count_matrix_nodes()
        {
            size_t c = 0;
            MatrixNode *current = this->child;
            while (current != nullptr)
            {
                c += current->count_matrix_nodes();
                current = current->next;
            }
            return c;
        }
    };
};

// We have to hold off on destructor definitions until here
template <IsStateTypes Types, typename MatrixStats, typename ChanceStats>
DefaultNodes<Types, MatrixStats, ChanceStats>::MatrixNode::~MatrixNode()
{
    while (this->child != nullptr)
    {
        DefaultNodes<Types, MatrixStats, ChanceStats>::ChanceNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
}
template <IsStateTypes Types, typename MatrixStats, typename ChanceStats>
DefaultNodes<Types, MatrixStats, ChanceStats>::ChanceNode::~ChanceNode()
{
    while (this->child != nullptr)
    {
        DefaultNodes<Types, MatrixStats, ChanceStats>::MatrixNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
};