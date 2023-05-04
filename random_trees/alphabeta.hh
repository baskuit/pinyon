#include "libsurskit/gambit.hh"
#include "tree/tree.hh"
#include "algorithm/algorithm.hh"

/*

An implementation of Simultaneous Move Alpha Beta

See "Using Double-Oracle Method and Serialized Alpha-Beta Search
for Pruning in Simultaneous Move Games"

*/

template <typename Model>
class AlphaBeta : public AbstractAlgorithm<Model>
{

    struct MatrixStats;
    struct ChanceStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = AlphaBeta::MatrixStats;
        using ChanceStats = AlphaBeta::ChanceStats;
    };
    struct MatrixStats
    {
        typename Types::MatrixReal p;
        typename Types::MatrixReal o;
        typename Types::Real alpha = min_val;
        typename Types::Real beta = max_val;
        typename Types::VectorInt I, J; // use raw indices
    } struct ChanceStats
    {
        typename Types::Probability explored = 0;
    }

    const typename Types::Real min_val = 0;
    const typename Types::Real max_val = 1;

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : min_val(min_val), max_val(max_val) {}

    typename Types::Real double_oracle(
        typename Types::State &state,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {

        typename Types::MatrixReal &p = matrix_node->stats.p;
        typename Types::MatrixReal &q = matrix_node->stats.q;


        // double oracle is only called on a state exactly once, so we can expand here
        state.get_actions()
        matrix_node.actions = state.actions;

        p;
        // set to minval
        o;
        // set to maxval

        // Solving a restricted matrix is done by creating a new one? It's basically only done one and then the matrix is expanded



        if (state.is_terminal) {
            return state.row_payoff;
        }

        while (alpha < beta) { // consider fuzzy eq

            for (int i : matrix_node.stats.I) {
                for (int j : matrix_node.stats.J) {
                    auto p_ij = p.get(i, j);
                    auto o_ij = o.get(i, j);

                    if (p_ij < o_ij) {
                        auto u_ij = double_oracle(s_ij, matrix_node_next p_ij, o_ij)
                        p_ij = o_ij = u_ij;
                    }

                }
            }

            // get restricted matrix of u values
            typename Types::MatrixReal M;

            // Calculate NE on restricted stage game
            typename Types::VectorReal row_strategy, col_strategy;
            Gambit::SolveMatrix(M, row_strategy, col_strategy);

            
        
        }

        return .5;
    }

    typename Types::Real best_response_row()
    {
        return .5;
    }

    typename Types::Real best_response_col()
    {
        return .5;
    }
};
