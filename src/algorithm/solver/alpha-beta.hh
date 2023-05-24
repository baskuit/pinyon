#include "../../libsurskit/gambit.hh"
#include "../../tree/tree.hh"
#include "../algorithm.hh"

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
        MatrixNode<FullTraversal<Model>> *teacher = nullptr;
        typename Types::VectorReal row_strategy, col_strategy;
        int row_br_idx, col_br_idx;
        bool ok = true;
        typename Types::MatrixInt function_calls;
        int total_calls = 0;
    }; 
    struct ChanceStats
    {
        typename Types::Probability explored = Rational(0);
    };

    const typename Types::Real min_val = 0;
    const typename Types::Real max_val = 1;
    int max_depth = -1;
    typename Types::Real epsilon = Rational(1, 1 << 24);

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : 
        min_val(min_val), max_val(max_val) {}

    void run (
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *root,
        MatrixNode<FullTraversal<Model>> *teacher
    ) {
        root->stats.teacher = teacher;
        double_oracle(state, model, root, min_val, max_val);
    }


    typename Types::Real double_oracle(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        state.get_actions();
        matrix_node->row_actions = state.row_actions;
        matrix_node->col_actions = state.col_actions;
        auto &stats = matrix_node->stats;

        // if s is terminal state then
        if (state.is_terminal) {
            matrix_node->is_terminal = true;
            stats.value = state.row_payoff;
            teacher_check(matrix_node);
            return state.row_payoff;
        }
        if (max_depth > 0 && stats.depth >= max_depth) {
            matrix_node->is_terminal = true;
            model.get_inference(state, matrix_node->inference);
            stats.value = matrix_node->inference.row_value;
            teacher_check(matrix_node);
            return matrix_node->inference.row_value;
        }

        // 6: initialize restricted action sets I and J with a first action in stage s
        std::vector<int> &I = stats.I;
        std::vector<int> &J = stats.J;
        // TODO first action using info from last expansion, if possible
        I.push_back(0);
        J.push_back(0);

        typename Types::MatrixReal &p = stats.p;
        typename Types::MatrixReal &o = stats.o;
        // 7: pI,J ← alpha-betaMin (sI,J , minval, maxval)
        p.fill(state.row_actions.size(), state.col_actions.size(), min_val);
        // 8: oI,J ← alpha-betaMax (sI,J , minval, maxval)
        o.fill(state.row_actions.size(), state.col_actions.size(), max_val);
        stats.function_calls.fill(state.row_actions.size(), state.col_actions.size(), 0);
        stats.total_calls = 1;
        // Note: this implementation does not use serialized alpha beta
        // Just seems like too much tree traversal?
        // 9: repeat, 23: until α = β
       
        while (!equals(alpha, beta)) {

            // 10: for i ∈ I, j ∈ J do
            for (int row_idx : stats.I) {
                for (int col_idx : stats.J) {

                    typename Types::Real &p_ij = p.get(row_idx, col_idx);
                    typename Types::Real &o_ij = o.get(row_idx, col_idx);
                    // 11: if pi, j < oi, j then
                    if (p_ij < o_ij) {
                        // 12: u(si, j ) ← double-oracle(si, j , pi, j, oi, j )
                        typename Types::Real u_ij = 0;
                        ChanceNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
                        ChanceNode<FullTraversal<Model>> *chance_node_teacher = stats.teacher->access(row_idx, col_idx);

                        const typename Types::Action row_action = state.row_actions[row_idx];
                        const typename Types::Action col_action = state.col_actions[col_idx];

                        std::vector<typename Types::Observation> chance_actions;
                        state.get_chance_actions(chance_actions, row_action, col_action);
                        for (const auto chance_action : chance_actions) {

                            auto state_copy = state;
                            state_copy.apply_actions(row_action, col_action, chance_action);
                            MatrixNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.transition);
                            matrix_node_next->stats.teacher = chance_node_teacher->access(state_copy.transition);

                            u_ij += double_oracle(state_copy, model, matrix_node_next, p_ij, o_ij) * state_copy.transition.prob;
                            // u_ij is the value of chance node, i.e. expected score over all child matrix nodes
                            stats.function_calls.get(row_idx, col_idx) += 1;

                        }
                        // 13: pi, j ← u(si, j ); oi, j ← u(si, j )
                        p_ij = u_ij;
                        o_ij = u_ij;
                    }
                }
            }

            // 14: <u(s), (x,y)> ← ComputeNE(I, J)
            typename Types::MatrixReal matrix;
            typename Types::VectorReal row_strategy, col_strategy;
            stats.value = solve_submatrix(matrix, matrix_node, row_strategy, col_strategy);
            // 15: hi0, vMaxi ← BRMax (s, α, y)
            auto iv = best_response_row(state, model, matrix_node, alpha, col_strategy);
            // 16: h j0 , vMini ← BRMin(s, β, x)
            auto jv = best_response_col(state, model, matrix_node, beta,  row_strategy);
                stats.row_br_idx = iv.first;
                stats.col_br_idx = jv.first;
                stats.row_strategy = row_strategy;
                stats.col_strategy = col_strategy;


            // 17 - 20
            if (iv.first == -1) {
                stats.value = min_val;
                teacher_check(matrix_node);
                return min_val;
            }
            if (jv.first == -1) {
                stats.value = max_val;
                teacher_check(matrix_node);
                return max_val;
            }
            // 21: α ← max(α, vMin); β ← min(β, v Max )
            alpha = std::max(alpha, jv.second);
            beta =  std::min(beta , iv.second);

            // 22: I ← I ∪ {i0}; J ← J ∪ { j0 }
            if (std::find(I.begin(), I.end(), iv.first) == I.end()) {
                I.push_back(iv.first);
            }
            if (std::find(J.begin(), J.end(), jv.first) == J.end()) {
                J.push_back(jv.first);
            }
            stats.value = alpha;
        }
        teacher_check(matrix_node);
        return stats.value;
    }

    void teacher_check(MatrixNode<AlphaBeta> *matrix_node) {
        auto &stats = matrix_node->stats;
        for (int entry_idx = 0; entry_idx < matrix_node->actions.rows * matrix_node->actions.cols; ++entry_idx) {
            stats.total_calls += stats.function_calls.data[entry_idx];
        }
        if (!equals(stats.value, stats.teacher->stats.row_payoff)) {
            // std::cout << "p:\n";
            // stats.p.print();
            // std::cout << "o:\n";
            // stats.o.print();
            // std::cout << "I:\n";
            // math::print(stats.I, stats.I.size());
            // std::cout << "J:\n";
            // math::print(stats.J, stats.J.size());
            // std::cout << "row_strategy:\n";
            // math::print(stats.row_strategy, stats.I.size());
            // std::cout << "col_strategy:\n";
            // math::print(stats.col_strategy, stats.J.size());
            // std::cout << "nash_payoff_matrix:\n";
            // auto npm = stats.teacher->stats.nash_payoff_matrix;
            // npm.print();
            // std::cout << "row_solution:\n";
            // math::print(stats.teacher->stats.row_solution, npm.rows);
            // std::cout << "col_solution:\n";
            // math::print(stats.teacher->stats.col_solution, npm.cols);


            stats.ok = false;
        }
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

        // 1: BRvalue ← α
        typename Types::Real best_response_row = alpha;
        // 2: i BR ← null
        int new_action_idx = -1;

        // 3: for i = {1, . . . , n} do
        for (int row_idx = 0; row_idx < state.row_actions.size(); ++row_idx) {
            bool cont = false;
            
            // 4: pi,J ← alpha-betaMin(si,J , minval, maxval)
            // 5: oi,J ← alpha-betaMax (si,J , minval, maxval)
            // We don't perform serialized alpha-beta
            // Furthermore, I'm not sure what this step does that lines 7, 8 in double_oracle don't

            typename Types::Real expected_o_payoff = 0;
            for (int j = 0; j < J.size(); ++j) {
                expected_o_payoff += col_strategy[j] * o.get(row_idx, J[j]);
            }

            // 6: for j ∈ J; y j > 0 ∧ pi, j < oi, j do
            for (int j = 0; j < J.size(); ++j) {
                int col_idx = J[j];
                typename Types::Real y = col_strategy[j];

                typename Types::Real &p_ij = p.get(row_idx, col_idx);
                typename Types::Real &o_ij = o.get(row_idx, col_idx);

                if (y > 0 && p_ij < o_ij) {
                    // 7: p0 i, j ← max pi, j , BRvalue − P j0 ∈Jr{ j} y j0 · oi, j0
                    typename Types::Real p__ij = std::max(p_ij, best_response_row - expected_o_payoff + y * o_ij);
                
                    // 8: if p0 > oi, j then
                    if (p__ij > o_ij) {
                        cont = true;
                        break;
                    } else {

                        // 11: u(s_ij) = double_oracle (s_ij, p_ij, o_ij)
                        typename Types::Real u_ij = 0;
                        ChanceNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
                        ChanceNode<FullTraversal<Model>> *chance_node_teacher = matrix_node->stats.teacher->access(row_idx, col_idx);
                        
                        const typename Types::Action row_action = state.row_actions[row_idx];
                        const typename Types::Action col_action = state.col_actions[col_idx];

                        std::vector<typename Types::Observation> chance_actions;
                        state.get_chance_actions(chance_actions, row_action, col_action);
                        for (const auto chance_action : chance_actions) {
                            auto state_copy = state;
                            state_copy.apply_actions(row_action, col_action, chance_action);
                            MatrixNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.transition);
                            matrix_node_next->stats.teacher = chance_node_teacher->access(state_copy.transition);
                            u_ij += double_oracle(state_copy, model, matrix_node_next, p_ij, o_ij) * state_copy.transition.prob;
                        }

                        // 12: pi, j ← u(si, j ); oi, j ← u(si, j)
                        p_ij = u_ij;
                        o_ij = u_ij;
                    }
                }
            }

            // 9: continue with next i
            if (cont) {
                continue;
            }

            // 13 - 14
            typename Types::Real expected_row_payoff = 0;
            for (int j = 0; j < J.size(); ++j) {
                expected_row_payoff += col_strategy[j] * o.get(row_idx, J[j]);
            } // o_ij = u_ij by now
            if (expected_row_payoff >= best_response_row || (new_action_idx == -1 && equals(expected_row_payoff, best_response_row))) {
                new_action_idx = row_idx;
                best_response_row = expected_row_payoff;
            }
        }

        std::pair<int, double> pair{new_action_idx, best_response_row};
        return pair;
    }

    std::pair<int, typename Types::Real> best_response_col(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real beta,
        typename Types::VectorReal &row_strategy)
    {
        typename Types::MatrixReal &p = matrix_node->stats.p;
        typename Types::MatrixReal &o = matrix_node->stats.o;
        std::vector<int> &I = matrix_node->stats.I;
        std::vector<int> &J = matrix_node->stats.J;

        typename Types::Real best_response_col = beta;
        int new_action_idx = -1;

        for (int col_idx = 0; col_idx < state.col_actions.size(); ++col_idx) {
            bool cont = false;

            typename Types::Real expected_p_payoff = 0;
            for (int i = 0; i < I.size(); ++i) {
                expected_p_payoff += row_strategy[i] * p.get(I[i], col_idx);
            }

            for (int i = 0; i < I.size(); ++i) {
                int row_idx = I[i];
                typename Types::Real x = row_strategy[i];

                typename Types::Real &p_ij = p.get(row_idx, col_idx);
                typename Types::Real &o_ij = o.get(row_idx, col_idx);

                if (x > 0 && p_ij < o_ij) {
                    typename Types::Real o__ij = std::min(o_ij, best_response_col - expected_p_payoff + x * p_ij);
                
                    if (o__ij < p_ij) {
                        cont = true;
                        break;
                    } else {
                        typename Types::Real u_ij = 0;
                        ChanceNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
                        ChanceNode<FullTraversal<Model>> *chance_node_teacher = matrix_node->stats.teacher->access(row_idx, col_idx);

                        const typename Types::Action row_action = state.row_actions[row_idx];
                        const typename Types::Action col_action = state.col_actions[col_idx];

                        std::vector<typename Types::Observation> chance_actions;
                        state.get_chance_actions(chance_actions, row_action, col_action);
                        for (const auto chance_action : chance_actions) {
                            auto state_copy = state;
                            state_copy.apply_actions(row_action, col_action, chance_action);
                            MatrixNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.transition);
                            matrix_node_next->stats.teacher = chance_node_teacher->access(state_copy.transition);
                            u_ij += double_oracle(state_copy, model, matrix_node_next, p_ij, o_ij) * state_copy.transition.prob;
                            chance_node->stats.explored += state_copy.transition.prob;
                        }

                        // 12: pi, j ← u(si, j ); oi, j ← u(si, j)
                        p_ij = u_ij;
                        o_ij = u_ij;
                    }
                }
            }

            if (cont) {
                continue;
            }

            typename Types::Real expected_col_payoff = 0;
            for (int i = 0; i < I.size(); ++i) {
                expected_col_payoff += row_strategy[i] * p.get(I[i], col_idx);
            }
            if (expected_col_payoff <= best_response_col || (new_action_idx == -1 && equals(expected_col_payoff, best_response_col))) {
                new_action_idx = col_idx;
                best_response_col = expected_col_payoff;
            }
        }

        std::pair<int, double> pair{new_action_idx, best_response_col};
        return pair;
    }

private:

    template <typename T>
    bool equals (T x, T y) {
        return std::abs(x - y) < epsilon;
    }

    typename Types::Real solve_submatrix(
        typename Types::MatrixReal &M,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy) 
    {
        M.fill(matrix_node->stats.I.size(), matrix_node->stats.J.size());
        row_strategy.fill(M.rows);
        col_strategy.fill(M.cols);
        int entry_idx = 0;
        for (const int row_idx : matrix_node->stats.I) {
            for (const int col_idx : matrix_node->stats.J) {
                M.data[entry_idx++] = matrix_node->stats.p.get(row_idx, col_idx);
                assert(matrix_node->stats.p.get(row_idx, col_idx) == matrix_node->stats.o.get(row_idx, col_idx));
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

    typename Types::Real row_alpha_beta(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        return max_val;
    }

    typename Types::Real col_alpha_beta(
        typename Types::State &state,
        Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        return min_val;
    }
    // Serialized AlphaBeta, TODO

};
