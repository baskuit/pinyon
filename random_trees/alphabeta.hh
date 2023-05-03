#include "libsurskit/gambit.hh"
#include "tree/tree.hh"
#include "algorithm/algorithm.hh"

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
        typename Types::VectorInt I, J;
    } struct ChanceStats
    {
        typename Types::Probability explored = 0;
    }

    const typename Types::Real min_val = 0;
    const typename Types::Real max_val = 1;

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : min_val(min_val), max_val(max_val) {}

    typename Types::Real double_oracle(
        typename Types::State state,
        typename Types::Real alpha,
        typename Types::Real beta)
    {

        if (state.is_terminal) {
            return state.row_payoff;
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
