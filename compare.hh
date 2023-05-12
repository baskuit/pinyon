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
        for (int row_idx = 0; row_idx < state.actions.rows; ++row_idx) {
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
            if (expected_row_payoff > best_response_row) {
                new_action_idx = row_idx;
                best_response_row = expected_row_payoff;
            }
        }

        std::pair<int, double> pair{new_action_idx, best_response_row};
        return pair;
    }