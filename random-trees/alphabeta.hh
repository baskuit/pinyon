#include "libsurskit/gambit.hh"
#include "tree/tree.hh"
#include "algorithm/algorithm.hh"

// #include <algorithm
#include <ranges>
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

    void run (
        typename Types::State &state,
        Model &model
    ) {
        MatrixNode<AlphaBeta> *root;
        double_oracle(state, model, root, min_val, max_val);
    }

    typename Types::Real double_oracle(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        typename Types::MatrixReal &p = matrix_node->stats.p;
        typename Types::MatrixReal &o = matrix_node->stats.o;
        std::vector<int> &I = matrix_node->stats.I;
        std::vector<int> &J = matrix_node->stats.J;


        // double oracle is only called on a state exactly once, so we can expand here
        state.get_actions();
        matrix_node->actions = state.actions;

        if (state.is_terminal) {
            matrix_node->is_terminal = true;
            matrix_node->stats.value = state.row_payoff;
            return state.row_payoff;
        }
        if (max_depth > 0 && matrix_node->stats.depth >= max_depth) {
            matrix_node->is_terminal = true;
            model.get_inference(state, matrix_node->inference);
            matrix_node->stats.value = matrix_node->inference.row_value;
            return matrix_node->inference.row_value;
        }

        p.fill(state.actions.rows, state.actions.cols, min_val);
        o.fill(state.actions.rows, state.actions.cols, max_val);

        while (alpha < beta) { // consider fuzzy eq

            for (int row_idx : matrix_node->stats.I) {
                for (int col_idx : matrix_node->stats.J) {

                    auto p_ij = p.get(row_idx, col_idx);
                    auto o_ij = o.get(row_idx, col_idx);
                    if (p_ij < o_ij) {
                        typename Types::Real u_ij = 0;
                        ChanceNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);

                        const typename Types::Action row_action = state.actions.row_actions[row_idx];
                        const typename Types::Action col_action = state.actions.col_actions[col_idx];

                        std::vector<typename Types::Observation> chance_actions;
                        state.get_chance_actions(chance_actions, row_action, col_action);
                        for (const auto chance_action : chance_actions) {

                            auto state_copy = state;
                            state_copy.apply_actions(row_action, col_action, chance_action);
                            MatrixNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.transition);
                            u_ij += double_oracle(state_copy, model, matrix_node_next, p_ij, o_ij) * state_copy.transition.prob;

                        }
                        p_ij = u_ij;
                        o_ij = u_ij;
                    }
                }
            }
            
            // Calculate NE on restricted stage game
            // we use an array here since its doesn't follow the same conventions as 'VectorReal'.
            // Namely VectorReal is a vector over the set of legal actions. This is a subset with likely different ordering.
            typename Types::VectorReal row_strategy, col_strategy;
            matrix_node->stats.value = solve_submatrix(matrix_node, row_strategy, col_strategy);

            auto iv = best_response_row(state, model, matrix_node, alpha, col_strategy);
            auto jv = best_response_col(state, model, matrix_node, beta,  row_strategy);
            if (iv.first < 0) {
                return min_val;
            }
            if (jv.first < 0) {
                return max_val;
            }
            alpha = std::max(alpha, jv.second);
            beta =  std::min(beta , iv.second);
            if (std::find(I.begin(), I.end(), iv.first) == I.end()) {
                I.push_back(iv.first);
            }
            if (std::find(J.begin(), J.end(), jv.first) == J.end()) {
                J.push_back(jv.first);
            }


        }

        return matrix_node->stats.value;
    }

    std::pair<int, typename Types::Real> best_response_row(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::VectorReal &col_strategy)
    {
        typename Types::MatrixReal &p = matrix_node->stats.p;
        typename Types::MatrixReal &o = matrix_node->stats.o;
        std::vector<int> &I = matrix_node->stats.I;
        std::vector<int> &J = matrix_node->stats.J;

        int new_action_idx = -1;
        typename Types::Real best_response_row = alpha;

        for (int row_idx = 0; row_idx < state.actions.rows; ++row_idx) {
            bool cont = false;
            
            // not returned
            typename Types::Real expected_row_payoff = 0;
            for (const auto& [col_idx, y] : std::views::zip(matrix_node->stats.J, col_strategy)) {
                expected_row_payoff += y * matrix_node->stats.o.get(row_idx, col_idx);
            }

            for (const auto& [col_idx, y] : std::views::zip(matrix_node->stats.J, col_strategy)) {
                typename Types::Real &p_ij = p.get(row_idx, col_idx);
                typename Types::Real &o_ij = o.get(row_idx, col_idx);

                if (y > 0 && p_ij < o_ij) {
                    typename Types::Real p__ij = std::max(
                        p_ij, best_response_row - expected_row_payoff + y * o_ij);
                
                    if (p__ij > o_ij) {
                        cont = true;
                        break;
                    } else {

                        // u(s_ij) = double_oracle (s_ij, p_ij, o_ij)
                        typename Types::Real u_ij = 0;
                        ChanceNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
                        
                        const typename Types::Action row_action = state.actions.row_actions[row_idx];
                        const typename Types::Action col_action = state.actions.col_actions[col_idx];

                        std::vector<typename Types::Observation> chance_actions;
                        state.get_chance_actions(chance_actions, row_action, col_action);
                        for (const auto chance_action : chance_actions) {
                            auto state_copy = state;
                            state_copy.apply_actions(row_action, col_action, chance_action);
                            MatrixNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.transition);
                            u_ij += double_oracle(state_copy, model, matrix_node_next, p_ij, o_ij) * state_copy.transition.prob;
                        }

                        p_ij = u_ij;
                        o_ij = u_ij;
                    }
                }
            }

            if (cont) {
                continue;
            }

            typename Types::Real expected_row_payoff_real = 0;
            for (const auto& [col_idx, y] : std::views::zip(matrix_node->stats.J, col_strategy)) {
                expected_row_payoff_real += y * matrix_node->stats.o.get(row_idx, col_idx);
            }
            if (expected_row_payoff_real > best_response_row) {
                new_action_idx = row_idx;
                best_response_row = expected_row_payoff_real;
            }
        }

        std::pair<int, double> pair{new_action_idx, best_response_row};
        return pair;
    }

    std::pair<int, typename Types::Real> best_response_col(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::VectorReal &col_strategy)
    {
        int new_action_idx = -1;
        typename Types::Real best_response_col = alpha;

        std::pair<int, double> pair{new_action_idx, best_response_col};
        return pair;
    }
private:

    typename Types::Real solve_submatrix(
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy) 
    {
        typename Types::MatrixReal M;
        M.fill(matrix_node->stats.I.size(), matrix_node->stats.J.size());
        int entry_idx = 0;
        for (const int row_idx : matrix_node->stats.I) {
            for (const int col_idx : matrix_node->stats.J) {
                M.data[entry_idx++] = matrix_node->stats.p.get(row_idx, col_idx);
                // we can use either p or q here since the substage is solved
            }
        }
        LibGambit::solve_matrix<Types>(M, row_strategy, col_strategy);
        typename Types::Real value = 0;
        for (int row_idx = 0; row_idx < M.rows; ++row_idx) {
            for (int col_idx = 0; col_idx < M.cols; ++col_idx) {
                value += M.get(row_idx, col_idx) * row_strategy[row_idx] * col_strategy[col_idx];
            }                
        }
        return value;
    }
};
