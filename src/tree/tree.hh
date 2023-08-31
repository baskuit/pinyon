#pragma once

#include <libsurskit/math.hh>
#include <state/state.hh>
#include <tree/node.hh>

template <CONCEPT(IsStateTypes, Types), typename MStats, typename CStats>
struct DefaultNodes : Types
{
    class MatrixNode;

    class ChanceNode;

    using MatrixStats = MStats;
    using ChanceStats = CStats;

    class MatrixNode
    {
    public:
        static constexpr bool STORES_VALUE = false;

        ChanceNode *child = nullptr;
        MatrixNode *next = nullptr;

        bool terminal = false;
        bool expanded = false;

        typename Types::Obs obs;
        MatrixStats stats;

        MatrixNode(){};
        MatrixNode(Types::Obs obs) : obs(obs) {}
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

        ChanceNode *access(int row_idx, int col_idx, Types::Mutex &mutex)
        {
            mutex.lock();
            if (this->child == nullptr)
            {
                this->child = new ChanceNode(row_idx, col_idx);
                mutex.unlock();
                return this->child;
            }
            ChanceNode *current = this->child;
            ChanceNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->row_idx == row_idx && current->col_idx == col_idx)
                {
                    mutex.unlock();
                    return current;
                }
                current = current->next;
            }
            ChanceNode *child = new ChanceNode(row_idx, col_idx);
            previous->next = child;
            mutex.unlock();
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

        ChanceNode &access_ref(int row_idx, int col_idx)
        {
            if (this->child == nullptr)
            {
                this->child = new ChanceNode(row_idx, col_idx);
                return *this->child;
            }
            ChanceNode *current = this->child;
            ChanceNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->row_idx == row_idx && current->col_idx == col_idx)
                {
                    return *current;
                }
                current = current->next;
            }
            ChanceNode *child = new ChanceNode(row_idx, col_idx);
            previous->next = child;
            return *child;
        }
    };

    // Chance Node
    class ChanceNode
    {
    public:
        MatrixNode *child = nullptr;
        ChanceNode *next = nullptr;

        int row_idx;
        int col_idx;

        ChanceStats stats;

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

        MatrixNode *access(const Types::Obs &obs, Types::Mutex &mutex)
        {
            mutex.lock();
            if (this->child == nullptr)
            {
                MatrixNode *child = new MatrixNode(obs);
                this->child = child;
                mutex.unlock();
                return child;
            }
            MatrixNode *current = this->child;
            MatrixNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->obs == obs)
                {
                    mutex.unlock();
                    return current;
                }
                current = current->next;
            }
            MatrixNode *child = new MatrixNode(obs);
            previous->next = child;
            mutex.unlock();
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

        MatrixNode &access_ref(const typename Types::Obs &obs)
        {
            if (this->child == nullptr)
            {
                MatrixNode *child = new MatrixNode(obs);
                this->child = child;
                return *child;
            }
            MatrixNode *current = this->child;
            MatrixNode *previous = this->child;
            while (current != nullptr)
            {
                previous = current;
                if (current->obs == obs)
                {
                    return *current;
                }
                current = current->next;
            }
            MatrixNode *child = new MatrixNode(obs);
            previous->next = child;
            return *child;
        };
    };
};

// We have to hold off on destructor definitions until here
template <CONCEPT(IsStateTypes, Types), typename MatrixStats, typename ChanceStats>
DefaultNodes<Types, MatrixStats, ChanceStats>::MatrixNode::~MatrixNode()
{
    while (this->child != nullptr)
    {
        DefaultNodes<Types, MatrixStats, ChanceStats>::ChanceNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
}
template <CONCEPT(IsStateTypes, Types), typename MatrixStats, typename ChanceStats>
DefaultNodes<Types, MatrixStats, ChanceStats>::ChanceNode::~ChanceNode()
{
    while (this->child != nullptr)
    {
        DefaultNodes<Types, MatrixStats, ChanceStats>::MatrixNode *victim = this->child;
        this->child = this->child->next;
        delete victim;
    }
};
