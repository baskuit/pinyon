#pragma once

#include <libsurskit/math.hh>
#include <state/state.hh>
#include <tree/node.hh>

#include <unordered_map>

template <CONCEPT(IsStateTypes, Types), typename MatrixStats, typename ChanceStats>
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

        typename Types::VectorAction row_actions;
        typename Types::VectorAction col_actions;
        typename Types::MatrixStats stats;

        ChanceNode **edges;

        MatrixNode(){};
        MatrixNode(Types::Obs obs) : obs(obs) {}
        ~MatrixNode();

        inline void expand(Types::State &state)
        {
            expanded = true;
            row_actions = state.row_actions;
            col_actions = state.col_actions;
            const size_t n_children = row_actions.size() * col_actions.size();
            edges = new ChanceNode *[n_children];
            std::fill_n(edges, n_children, nullptr);
        }

        void apply_actions(Types::State &state, const int row_idx, const int col_idx) const
        {
            state.apply_actions(row_actions[row_idx], col_actions[col_idx]);
        }

        Types::Action get_row_action(const int row_idx) const
        {
            return row_actions[row_idx];
        }

        Types::Action get_col_action(const int col_idx) const
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

        inline void get_value(Types::Value &value)
        {
        }

        ChanceNode *access(int row_idx, int col_idx)
        {
            const int child_idx = row_idx * col_actions.size() + col_idx;
            const ChanceNode *&child = edges[child_idx]; // ref to pointer
            if (child == nullptr)
            {
                child = new ChanceNode();
            }
            return child;
        };

        const ChanceNode *access(int row_idx, int col_idx) const
        {
            const int child_idx = row_idx * col_actions.size() + col_idx;
            const ChanceNode *&child = edges[child_idx];
            return child;
        };

        size_t count_matrix_nodes()
        {
            size_t c = 1;
            const size_t n_children = row_actions.size() * col_actions.size();

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

        ChanceNode() {}
        ~ChanceNode();

        MatrixNode *access(Types::Obs &obs)
        {
            MatrixNode *&child = edges[obs];
            if (child == nullptr)
            {
                child = new MatrixNode(obs);
                return child;
            }
            return child;
        };

        const MatrixNode *access(Types::Obs &obs) const
        {
            return edges[obs];
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
