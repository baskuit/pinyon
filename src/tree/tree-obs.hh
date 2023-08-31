#pragma once

#include <libsurskit/math.hh>
#include <state/state.hh>
#include <tree/node.hh>

template <CONCEPT(IsStateTypes, Types), typename MStats, typename CStats>
struct LNodes : Types
{
    /*
    Instead of MatrixNodes storing their obs memeber, they are bundled with 'Edge' struct
    which is basically a linked list that the chance node parent owns
    */

    class MatrixNode;

    class ChanceNode;

    using MatrixStats = MStats;
    using ChanceStats = CStats;

    class MatrixNode
    {
    public:
        static constexpr bool STORES_VALUE = false;

        ChanceNode *child = nullptr;

        bool terminal = false;
        bool expanded = false;

        typename Types::MatrixStats stats;

        MatrixNode(){};
        ~MatrixNode();

        inline void expand(const size_t &, const size_t &)
        {
            expanded = true;
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

        inline void get_value(Types::Value &value) const
        {
        }

        ChanceNode *access(int row_idx, int col_idx)
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

        const ChanceNode *access(int row_idx, int col_idx) const
        {
            if (this->child == nullptr)
            {
                return this->child;
            }
            const ChanceNode *current = this->child;
            const ChanceNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->row_idx == row_idx && current->col_idx == col_idx)
                {
                    return current;
                }
                current = current->next;
            }
            return current;
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

        int row_idx;
        int col_idx;

        typename Types::ChanceStats stats;

        ChanceNode() {}
        ChanceNode(
            int row_idx,
            int col_idx) : row_idx(row_idx), col_idx(col_idx) {}
        ~ChanceNode();

        MatrixNode *access(const Types::Obs &obs)
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

        const MatrixNode *access(const Types::Obs &obs) const
        {
            if (this->child == nullptr)
            {
                return this->child;
            }
            const MatrixNode *current = this->child;
            const MatrixNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->obs == obs)
                {
                    return current;
                }
                current = current->next;
            }
            return current;
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

template <CONCEPT(IsStateTypes, Types), typename MatrixStats, typename ChanceStats>
LNodes<Types, MatrixStats, ChanceStats>::MatrixNode::~MatrixNode()
{
    while (this->child != nullptr)
    {
        LNodes<Types, MatrixStats, ChanceStats>::ChanceNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
}
template <CONCEPT(IsStateTypes, Types), typename MatrixStats, typename ChanceStats>
LNodes<Types, MatrixStats, ChanceStats>::ChanceNode::~ChanceNode()
{
    while (this->child != nullptr)
    {
        LNodes<Types, MatrixStats, ChanceStats>::MatrixNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
};