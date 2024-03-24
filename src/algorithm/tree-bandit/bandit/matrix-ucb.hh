#pragma once

#include <assert.h>

#include <algorithm/algorithm.hh>
#include <concepts>
#include <libpinyon/lrslib.hh>
#include <libpinyon/math.hh>
#include <ranges>
#include <tree/tree.hh>
#include <types/matrix.hh>

/*
MatrixUCB (Uses usual UCB formula, no time estimates)
*/

template <IsValueModelTypes Types>
struct MatrixUCB : Types {
   public:
    struct MatrixStats {
        int total_visits = 0;
        Types::MatrixReal row_value_matrix;
        Types::MatrixReal col_value_matrix;
        Types::MatrixInt visit_matrix;

        Types::VectorReal row_strategy;
        Types::VectorReal col_strategy;
    };

    struct ChanceStats {};

    struct Outcome {
        int row_idx, col_idx;
        Types::Value value;
        Types::Real row_mu, col_mu;
    };

    class BanditAlgorithm {
        public:
        using Real = typename Types::Real;

        BanditAlgorithm() {}

        BanditAlgorithm(Types::Real) {}

        BanditAlgorithm(Types::Real c_uct, Types::Real expl_threshold) : c_uct(c_uct), expl_threshold(expl_threshold) {}

        friend std::ostream &operator<<(std::ostream &os, const BanditAlgorithm &session) {
            os << "MatrixUCB; c_uct: " << session.c_uct << ", expl_threshold: " << session.expl_threshold;
            return os;
        }

        const Types::Real c_uct{2};
        const Types::Real expl_threshold{.005};
        // bool require_interior = false;

        void initialize_stats(int playouts, Types::State &state, Types::Model &model, MatrixStats &stats) {}

        void get_empirical_strategies(const MatrixStats &stats, Types::VectorReal &row_strategy,
                                      Types::VectorReal &col_strategy) const {
            const int rows = stats.visit_matrix.rows;
            const int cols = stats.visit_matrix.cols;
            row_strategy.resize(rows);
            col_strategy.resize(cols);
            for (int row_idx = 0; row_idx < rows; ++row_idx) {
                for (int col_idx = 0; col_idx < cols; ++col_idx) {
                    const int n = stats.visit_matrix.get(row_idx, col_idx);
                    row_strategy[row_idx] += n;
                    col_strategy[col_idx] += n;
                }
            }
            math::power_norm(row_strategy, rows, 1, row_strategy);
            math::power_norm(col_strategy, cols, 1, col_strategy);
        }

        // void get_refined_strategies(MatrixNode<MatrixUCB> *matrix_node, typename Types::VectorReal &row_strategy,
        //                     typename Types::VectorReal &col_strategy) {
        //     typename Types::MatrixReal row_ev_matrix(matrix_node->actions.rows, matrix_node->actions.cols);
        //     typename Types::MatrixReal col_ev_matrix(matrix_node->actions.rows, matrix_node->actions.cols);
        //     get_ev_matrix(matrix_node, row_ev_matrix, col_ev_matrix);
        //     LibGambit::solve_bimatrix<Types>(row_ev_matrix, col_ev_matrix, row_strategy, col_strategy);
        // }

        void expand(MatrixStats &stats, const size_t &rows, const size_t &cols,
                    const Types::ModelOutput &output) const {  // matrix_node->is_expanded = true;
            stats.row_value_matrix.fill(rows, cols, 0);
            stats.col_value_matrix.fill(rows, cols, 0);
            stats.visit_matrix.fill(rows, cols, 0);

            // Uniform initialization of stats.strategies

            stats.row_strategy.resize(rows, 1 / static_cast<Real>(rows));
            stats.col_strategy.resize(cols, 1 / static_cast<Real>(cols));
        }

        void select(Types::PRNG &device, MatrixStats &stats, Outcome &outcome) const {
            const size_t rows = stats.visit_matrix.rows;
            const size_t cols = stats.visit_matrix.cols;
            typename Types::template Matrix<PairReal<Real>> ucb_matrix{rows, cols};
            get_ucb_matrix(stats);
            for (int row_idx{}; row_idx < rows; ++row_idx) {
                for (int col_idx{}; col_idx < cols; ++col_idx) {
                    auto &pair = ucb_matrix.get(row_idx, col_idx);
                    pair.row_value = stats.row_value_matrix.get(row_idx, col_idx);
                    pair.col_value = stats.col_value_matrix.get(row_idx, col_idx);
                }
            }
            typename Types::VectorReal &row_strategy = stats.row_strategy;
            typename Types::VectorReal &col_strategy = stats.col_strategy;
            Real expl = math::exploitability(stats.row_value_matrix, stats.col_value_matrix, row_strategy, col_strategy);
            if (expl > expl_threshold) {
                LRSNash::solve(ucb_matrix, row_strategy, col_strategy);
            }
            const int row_idx = device.sample_pdf(row_strategy);
            const int col_idx = device.sample_pdf(col_strategy);
            outcome.row_idx = row_idx;
            outcome.col_idx = col_idx;
        }

        void update_matrix_stats(MatrixStats &stats, Outcome &outcome) const {
            ++stats.total_visits;
            stats.row_value_matrix.get(outcome.row_idx, outcome.col_idx) += outcome.value.get_row_value();
            stats.col_value_matrix.get(outcome.row_idx, outcome.col_idx) += outcome.value.get_col_value();
            stats.visit_matrix.get(outcome.row_idx, outcome.col_idx) += 1;
        }

        void update_chance_stats(ChanceStats &stats, Outcome &outcome) const {}

        // private:
        void get_ucb_matrix(MatrixStats &stats) const {
            typename Types::MatrixReal &row_value_matrix = stats.row_value_matrix;
            typename Types::MatrixReal &col_value_matrix = stats.col_value_matrix;
            const typename Types::MatrixInt &visit_matrix = stats.visit_matrix;
            const int rows = visit_matrix.rows;
            const int cols = visit_matrix.cols;
            const typename Types::Real log_total = std::log(stats.total_visits);
            for (int row_idx = 0; row_idx < rows; ++row_idx) {
                for (int col_idx = 0; col_idx < cols; ++col_idx) {
                    const typename Types::Real u = row_value_matrix.get(row_idx, col_idx);
                    const typename Types::Real v = col_value_matrix.get(row_idx, col_idx);
                    int n = visit_matrix.get(row_idx, col_idx);
                    n += (n == 0);
                    typename Types::Real a = u / n;
                    typename Types::Real b = v / n;
                    typename Types::Real const eta = this->c_uct * std::sqrt(log_total / n);
                    const typename Types::Real x = a + eta;
                    const typename Types::Real y = b + eta;
                    row_value_matrix.get(row_idx, col_idx) = x;
                    col_value_matrix.get(row_idx, col_idx) = y;
                }
            }
        }

        // only used in get_refined, which is also commented out
        // void get_ev_matrix(MatrixNode<MatrixUCB> *matrix_node, typename Types::MatrixReal &row_ev_matrix,
        //                    typename Types::MatrixReal &col_ev_matrix) {
        //     typename Types::MatrixReal &row_value_matrix = matrix_node->stats.row_value_matrix;
        //     typename Types::MatrixReal &col_value_matrix = matrix_node->stats.col_value_matrix;
        //     typename Types::MatrixInt &visit_matrix = matrix_node->stats.visit_matrix;
        //     const int rows = visit_matrix.rows;
        //     const int cols = visit_matrix.cols;
        //     for (int row_idx = 0; row_idx < rows; ++row_idx) {
        //         for (int col_idx = 0; col_idx < cols; ++col_idx) {
        //             const typename Types::Real u = row_value_matrix.get(row_idx, col_idx);
        //             const typename Types::Real v = col_value_matrix.get(row_idx, col_idx);
        //             int n = visit_matrix.get(row_idx, col_idx);
        //             n += (n == 0);
        //             typename Types::Real a = u / n;
        //             typename Types::Real b = v / n;
        //             row_ev_matrix.get(row_idx, col_idx) = a;
        //             col_ev_matrix.get(row_idx, col_idx) = b;
        //         }
        //     }
        // }
    };
};