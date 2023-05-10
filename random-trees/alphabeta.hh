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
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = AlphaBeta::MatrixStats;
        using ChanceStats = AlphaBeta::ChanceStats;
    };
    struct MatrixStats
    {
        typename Types::Real value;
        // value for the maximizing/row player.
        typename Types::MatrixReal p;
        typename Types::MatrixReal o;
        // matrices of pessimistic/optimisitic values. always over full actions
        typename Types::Real alpha = min_val;
        typename Types::Real beta  = max_val;
        std::vector<int> I{}, J{}; 
        // vector of row_idx, col_idx in the substage
        int depth = 0;
    }; 
    struct ChanceStats
    {
        typename Types::Probability explored = Rational(0, 1);
    };

    const typename Types::Real min_val = 0;
    const typename Types::Real max_val = 1;
    int max_depth = -1;

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : 
        min_val(min_val), max_val(max_val) {}

    typename Types::Real double_oracle(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        typename Types::MatrixReal &p = matrix_node->stats.p;
        typename Types::MatrixReal &o = matrix_node->stats.q;
        typename Types::VectorInt &I = matrix_node->stats.I;
        typename Types::VectorInt &J = matrix_node->stats.J;


        // double oracle is only called on a state exactly once, so we can expand here
        state.get_actions();
        matrix_node.actions = state.actions;

        p.fill(state.actions.rows, state.actions.cols, min_val);
        // set to minval
        o.fill(state.actions.rows, state.actions.cols, max_val);
        // set to maxval

        if (state.is_terminal) {
            matrix_node->is_terminal = true;
            matrix_node->stats.value = state.row_payoff;
            return state.row_payoff;
        }
        if (max_depth > 0 && matrix_node->stats.depth >= max_depth) {
            matrix_node->is_terminal = true;
            model.inference(state, matrix_node->inference);
            matrix_node->stats.value = matrix_node->inference.row_value;
            return matrix_node->inference.row_value;
        }

        while (alpha < beta) { // consider fuzzy eq

            for (int row_idx : matrix_node.stats.I) {
                for (int col_idx : matrix_node.stats.J) {
                    auto p_ij = p.get(row_idx, col_idx);
                    auto o_ij = o.get(row_idx, col_idx);

                    if (p_ij < o_ij) {
                        typename Types::Action row_action = state.actions.row_actions[row_idx];
                        typename Types::Action col_action = state.actions.col_actions[col_idx];
                        ChanceNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
                        // recurse here
                        typename Types::Real u_ij = 0;
                        // iterate over all possible transitions
                        std::vector<typename Types::Observation> chance_actions;
                        state.get_chance_actions(chance_actions, row_action, col_action);
                        for (const auto chance_action : chance_actions) {
                            auto state_copy = state;
                            state_copy.apply_actions(row_action, col_action, chance_action);
                            MatrixNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.transition);
                            u_ij += double_oracle(state_copy, model, matrix_node_next, p_ij, o_ij) * state_copy.transitions.prob;
                        }
                        
                        p_ij = o_ij = u_ij;
                    }
                }
            }

            // get restricted matrix of u values
            typename Types::MatrixReal M;
            get_submatrix(M, matrix_node);

            // Calculate NE on restricted stage game
            std::vector<typename Types::Real> row_strategy, col_strategy;
            LibGambit::solve_matrix(M, row_strategy, col_strategy);
            // value = 0; // row payoff for these strategies

            // auto iv = best_response_row(state, model, matrix_node, alpha, col_strategy);
            // // auto jv = best_response_col(state, model, beta,  row_strategy);
            // if (iv[0] < 0) {
            //     return min_val;
            // } else {
            //     // add action if not in I
            // }
            // if (iv[0] < 0) {
            //     return max_val;
            // } else {
            //     // add action if not in J
            // }
            // if (jv[1] > alpha) {
            //     alpha = jv[1];
            // }
            // if (iv[1] < beta) {
            //     beta = iv[1];
            // }
        }
        return matrix_node->stats.value;
    }

    // std::pair<int, typename Types::Real> best_response_row(
    //     typename Types::State &state,
    //     Model &model,
    //     MatrixNode<AlphaBeta> *matrix_node,
    //     typename Types::Real alpha,
    //     std::vector<typename Types::Real> &col_strategy)
    // {
    //     typename Types::MatrixReal &p = matrix_node->stats.p;
    //     typename Types::MatrixReal &q = matrix_node->stats.q;
    //     typename Types::VectorInt &I = matrix_node->stats.I;
    //     typename Types::VectorInt &J = matrix_node->stats.J;

    //     int new_action_idx = -1;
    //     typename Types::Real best_response_value = alpha;

    //     for (int row_idx = 0; row_idx < state.actions.rows; ++row_idx) {
    //         bool cont = false;
            
    //         typename Types::Real expected_row_payoff = 0;
    //         for (const auto& [col_idx, y] : std::zip(matrix_node->stats.I, col_strategy)) {
    //             expected_row_payoff += y * matrix_node->stats.o.get(row_idx, col_idx);
    //         }

    //         for (const auto& [col_idx, y] : std::zip(matrix_node->stats.I, col_strategy)) {
    //             typename Types::Real &p_ij = p.get(row_idx, col_idx);
    //             typename Types::Real &o_ij = o.get(row_idx, col_idx);

    //             if (y > 0 && p_ij < o_ij) {
    //                 typename Types::Real p__ij = std::max(
    //                     p_ij, best_response_value - expected_row_payoff - matrix_node->stats.o.get(row_idx, col_idx));
                
    //                 if (p__ij > matrix_node->stats.o.get(row_idx, col_idx)) {
    //                     cont = true;
    //                     break;
    //                 } else {
    //                     auto state_copy = state;
    //                     matrix_node->
    //                 }
    //             }
                

    //         }
    //         if (cont) {
    //             continue
    //         }


    //     }

    //     std::pair<int, double> pair{new_action_idx, best_};

    //     return pair;
    // }

    // std::pair<int, typename Types::Real> best_response_col()
    // {
    //     return pair;
    // }
private:
    void get_submatrix(
        typename Types::MatrixReal &M,
        MatrixNode<AlphaBeta> *matrix_node) {
            M.fill(matrix_node->stats.I.size(), matrix_node->stats.J.size());
            int entry_idx = 0;
            for (const int row_idx : matrix_node->stats.I) {
                for (const int col_idx : matrix_node->stats.J) {
                    M.data[entry_idx++] = matrix_node->stats.p.get(row_idx, col_idx);
                    // we can use either p or q here since the substage is solved
                }
            }
        }
};
