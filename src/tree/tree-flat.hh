#pragma once

#include <libsurskit/math.hh>
#include <state/state.hh>
#include <tree/node.hh>

#include <unordered_map>

template <CONCEPT(IsStateTypes, Types), typename MStats, typename CStats>
struct FlatNodes : Types
{
    class MatrixNode;

    class ChanceNode;

    using MatrixStats = MStats;
    using ChanceStats = CStats;

    class MatrixNode
    {
    public:
        static constexpr bool STORES_VALUE = false;

        bool terminal = false;
        bool expanded = false;
        typename Types::Obs obs;
        int rows = 0;
        int cols = 0;

        typename Types::MatrixStats stats;

        ChanceNode **edges;

        MatrixNode(){};
        MatrixNode(Types::Obs obs) : obs(obs) {}
        ~MatrixNode();

        inline void expand(const Types::State &state)
        {
            expanded = true;
            rows = state.row_actions.size();
            cols = state.col_actions.size();
            const size_t n_children = rows * cols;
            edges = new ChanceNode *[n_children]{};
            // std::fill_n(edges, n_children, nullptr); // TODO does this work lol
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

        inline void get_value(Types::Value &value)
        {
        }

        ChanceNode *access(int row_idx, int col_idx)
        {
            const int child_idx = row_idx * cols + col_idx;
            const ChanceNode *&child = edges[child_idx]; // ref to pointer
            if (child == nullptr)
            {
                child = new ChanceNode();
            }
            return child;
        };

        const ChanceNode *access(int row_idx, int col_idx) const
        {
            const int child_idx = row_idx * cols + col_idx;
            const ChanceNode *&child = edges[child_idx];
            return child;
        };

        ChanceNode *access(int row_idx, int col_idx, Types::Mutex &mutex)
        {
            const int child_idx = row_idx * cols + col_idx;
            const ChanceNode *&child = edges[child_idx]; // ref to pointer
            mutex.lock();
            if (child == nullptr)
            {
                child = new ChanceNode();
            }
            mutex.unlock();
            return child;
        };

        size_t count_matrix_nodes() const
        {
            size_t c = 1;
            const size_t n_children = rows * cols;

            for (size_t i = 0; i < n_children; ++i)
            {
                ChanceNode *&chance_node = edges[i];
                if (edges[i] != nullptr)
                {
                    c += chance_node->count_matrix_nodes();
                }
            }
            return c;
        }
    };

    class ChanceNode
    {
    public:
        std::unordered_map<typename Types::Obs, MatrixNode *, typename Types::ObsHash> edges{};
        typename Types::ChanceStats stats{};

        ~ChanceNode();

        MatrixNode *access(Types::Obs &obs)
        {
            MatrixNode *&child = edges[obs];
            if (child == nullptr)
            {
                child = new MatrixNode(obs);
            }
            return child;
        };

        const MatrixNode *access(Types::Obs &obs) const
        {
            return edges[obs];
        };
    
        MatrixNode *access(Types::Obs &obs, Types::Mutex &mutex)
        {
            MatrixNode *&child = edges[obs];
            if (child == nullptr)
            {
                child = new MatrixNode(obs);
            }
            return child;
        };

        size_t count_matrix_nodes() const
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
};

template <CONCEPT(IsStateTypes, Types), typename MatrixStats, typename ChanceStats>
FlatNodes<Types, MatrixStats, ChanceStats>::MatrixNode::~MatrixNode()
{
    delete[] edges;
}

template <CONCEPT(IsStateTypes, Types), typename MatrixStats, typename ChanceStats>
FlatNodes<Types, MatrixStats, ChanceStats>::ChanceNode::~ChanceNode()
{
    for (const auto &[obs, matrix_node] : edges)
    {
        delete matrix_node;
    }
};
