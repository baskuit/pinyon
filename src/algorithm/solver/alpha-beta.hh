#include <libsurskit/lrslib.hh>
#include <types/matrix.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

// #include <algorithm
#include <ranges>
#include <concepts>
#include <assert.h>

template <CONCEPT(IsValueModelTypes, Types), template <typename...> typename NodePair = DefaultNodes>
struct AlphaBeta : Types
{
    struct Data
    {
        Types::Prob unexplored{typename Types::Q{1}};
        Types::Real alpha_explored{0}, beta_explored{0};
        int next_chance_idx = 0;
        std::vector<typename Types::Obs> chance_actions{};
    };
    struct MatrixStats
    {
        DataMatrix<Data> data_matrix{};
        Types::VectorReal row_solution{}, col_solution{};
        unsigned int depth = 0;

        int row_pricipal_idx = 0, col_pricipal_idx = 0;
        std::vector<int> I{}, J{};
    };
    struct ChanceStats
    {
    };
    using MatrixNode = NodePair<Types, MatrixStats, ChanceStats>::MatrixNode;
    using ChanceNode = NodePair<Types, MatrixStats, ChanceStats>::ChanceNode;

    class Search
    {
    public:
        NodePair<
            Types,
            typename FullTraversal<Types>::MatrixStats,
            typename FullTraversal<Types>::ChanceStats>::MatrixNode *teacher;

        const typename Types::Real min_val{Rational<>{0}}; // don't need to use the Game values if you happen to know that State's
        const typename Types::Real max_val{Rational<>{1}};
        bool (*const terminate)(typename Types::PRNG &, const Data &) = &dont_terminate;
        int max_depth = -1;

        Search() {}

        Search(typename Types::Real min_val, typename Types::Real max_val) : min_val(min_val), max_val(max_val) {}

        Search(
            Types::Real min_val, typename Types::Real max_val,
            bool (*const terminate)(typename Types::PRNG &, const Data &)) : min_val(min_val), max_val(max_val), terminate{terminate} {}

        auto run(
            Types::PRNG &device,
            const typename Types::State &state,
            typename Types::Model &model,
            MatrixNode &root)
        {
            auto state_copy = state;
            return double_oracle(device, state_copy, model, &root, min_val, max_val);
        }

        void run(
            size_t ms,
            Types::PRNG &device,
            const typename Types::State &state,
            typename Types::Model &model,
            MatrixNode &root)
        {
            model.inference();
        }

        std::pair<typename Types::Real, typename Types::Real>
        double_oracle(
            Types::PRNG &device,
            Types::State &state,
            typename Types::Model &model,
            MatrixNode *matrix_node,
            Types::Real alpha,
            Types::Real beta)
        {
            MatrixStats &stats = matrix_node->stats;
            std::vector<int> &I = stats.I;
            std::vector<int> &J = stats.J;
            typename Types::VectorReal &row_strategy = stats.row_solution;
            typename Types::VectorReal &col_strategy = stats.col_solution;

            state.get_actions();
            matrix_node->expand(state);

            if (state.is_terminal())
            {
                matrix_node->set_terminal();
                return {state.payoff.get_row_value(), state.payoff.get_row_value()};
            }
            if (max_depth > 0 && stats.depth >= max_depth)
            {
                matrix_node->set_terminal();
                typename Types::ModelOutput model_output;
                model.inference(state, model_output);
                return {model_output.value.get_row_value(), model_output.value.get_row_value()};
            }

            // here is the only place we can use prior solve info

            I.push_back(stats.row_pricipal_idx);
            J.push_back(stats.col_pricipal_idx);
            bool solved_exactly = true;

            const size_t rows = state.row_actions.size();
            const size_t cols = state.col_actions.size();

            // stats.data_matrix.fill(rows, cols);
            DataMatrix<Data> new_data_matrix{rows, cols}; // TODO
            stats.data_matrix = new_data_matrix;

            bool smaller_bounds = false;
            bool new_action = true;

            int latest_row_idx = stats.row_pricipal_idx;
            int latest_col_idx = stats.col_pricipal_idx;

            while (!fuzzy_equals(alpha, beta) && (smaller_bounds || new_action))
            {

                // get entry values/bounds for newly expanded sub game, use last added actions

                for (const int row_idx : I)
                {
                    Data &data = stats.data_matrix.get(row_idx, latest_col_idx);
                    ChanceNode *chance_node = matrix_node->access(row_idx, latest_col_idx);
                    solved_exactly &= try_solve_chance_node(device, state, model, matrix_node, row_idx, latest_col_idx);
                }

                for (const int col_idx : J)
                {
                    Data &data = stats.data_matrix.get(latest_row_idx, col_idx);
                    ChanceNode *chance_node = matrix_node->access(latest_row_idx, col_idx);
                    solved_exactly &= try_solve_chance_node(device, state, model, matrix_node, latest_row_idx, col_idx);
                }

                // solve newly expanded and explored game

                int entry_idx = 0;
                if (solved_exactly)
                {
                    typename Types::MatrixValue matrix{I.size(), J.size()};
                    for (auto row_idx : I)
                    {
                        for (auto col_idx : J)
                        {
                            const Data &data = stats.data_matrix.get(row_idx, col_idx);
                            matrix[entry_idx] = data.alpha_explored;
                            ++entry_idx;
                        }
                    }
                    LRSNash::solve(matrix, row_strategy, col_strategy);
                }
                else
                {
                    typename Types::MatrixValue alpha_matrix{I.size(), J.size()}, beta_matrix{I.size(), J.size()};
                    for (auto row_idx : I)
                    {
                        for (auto col_idx : J)
                        {
                            const Data &data = stats.data_matrix.get(row_idx, col_idx);
                            alpha_matrix[entry_idx] = static_cast<typename Types::Real>(data.alpha_explored + data.unexplored * min_val);
                            beta_matrix[entry_idx] = static_cast<typename Types::Real>(data.beta_explored + data.unexplored * max_val);
                            ++entry_idx;
                        }
                    }
                    typename Types::VectorReal temp;
                    LRSNash::solve(alpha_matrix, row_strategy, temp);
                    temp.clear();
                    LRSNash::solve(beta_matrix, temp, col_strategy);
                }

                std::pair<int, typename Types::Real>
                    iv = best_response_row(
                        device,
                        state,
                        model,
                        matrix_node,
                        alpha, max_val,
                        col_strategy);

                std::pair<int, typename Types::Real>
                    jv = best_response_col(
                        device,
                        state,
                        model,
                        matrix_node,
                        min_val, beta,
                        row_strategy);

                // prune this node if no best response is as good as alpha/beta
                if (iv.first == -1)
                {
                    return {min_val, min_val};
                }
                if (jv.first == -1)
                {
                    return {max_val, max_val};
                }

                smaller_bounds = false;
                new_action = false;
                latest_row_idx = iv.first;
                latest_col_idx = jv.first;

                if (std::find(I.begin(), I.end(), latest_row_idx) == I.end())
                {
                    I.push_back(latest_row_idx);
                    new_action = true;
                }
                if (std::find(J.begin(), J.end(), latest_col_idx) == J.end())
                {
                    J.push_back(latest_col_idx);
                    new_action = true;
                }
                if (jv.second > alpha)
                {
                    alpha = jv.second;
                    smaller_bounds = true;
                }
                if (iv.second < beta)
                {
                    beta = iv.second;
                    smaller_bounds = true;
                }
            }

            // stats.row_solution = row_strategy;
            // stats.col_solution = col_strategy;
            if (max_depth > 0)
            {
                stats.row_pricipal_idx = I[std::distance(row_strategy.begin(), std::max_element(row_strategy.begin(), row_strategy.end()))];
                stats.col_pricipal_idx = J[std::distance(col_strategy.begin(), std::max_element(col_strategy.begin(), col_strategy.end()))];
            }
            I.clear();
            J.clear();
            row_strategy.clear();
            col_strategy.clear();
            stats.data_matrix.clear();

            // I.resize(0);
            // J.resize(0);
            // row_strategy.resize(0);
            // col_strategy.resize(0);
            // stats.data_matrix.resize(0);

            return {alpha, beta};
        }

        std::pair<int, typename Types::Real>
        best_response_row(
            Types::PRNG &device,
            const typename Types::State &state,
            typename Types::Model &model,
            MatrixNode *matrix_node,
            Types::Real alpha, typename Types::Real beta,
            Types::VectorReal &col_strategy)
        {
            MatrixStats &stats = matrix_node->stats;
            std::vector<int> &I = stats.I;
            std::vector<int> &J = stats.J;
            int best_row_idx = -1;

            for (int row_idx = 0; row_idx < state.row_actions.size(); ++row_idx)
            {
                bool row_idx_in_I = (std::find(I.begin(), I.end(), row_idx) != I.end());
                const typename Types::Action row_action = state.row_actions[row_idx];

                typename Types::Real max_priority{0}, expected_score{0}, total_unexplored{0};
                std::vector<typename Types::Real> priority_scores;
                int col_idx, best_i;
                for (int i = 0; i < J.size(); ++i)
                {
                    const int col_idx_temp = J[i];
                    Data &data = stats.data_matrix.get(row_idx, col_idx_temp);
                    expected_score += col_strategy[i] * data.beta_explored;
                    typename Types::Real priority;
                    if (row_idx_in_I)
                    {
                        priority = typename Types::Q{0};
                    }
                    else
                    {
                        priority = col_strategy[i] * data.unexplored;
                    }
                    total_unexplored += priority;
                    priority_scores.push_back(priority);
                    if (priority > max_priority)
                    {
                        col_idx = col_idx_temp;
                        max_priority = priority;
                        best_i = i;
                    }
                }

                while (
                    (max_priority > typename Types::Real{Rational<>{0}}) &&
                    (typename Types::Real{expected_score + beta * total_unexplored} >= alpha))
                {
                    Data &data = stats.data_matrix.get(row_idx, col_idx);
                    const typename Types::Action col_action = state.col_actions[col_idx];
                    if (data.chance_actions.size() == 0)
                    {
                        state.get_chance_actions(row_action, col_action, data.chance_actions);
                    }
                    if (data.next_chance_idx >= data.chance_actions.size())
                    {
                        break;
                    }
                    typename Types::State state_copy = state;
                    state_copy.apply_actions(row_action, col_action, data.chance_actions[data.next_chance_idx++]);
                    ChanceNode *chance_node = matrix_node->access(row_idx, col_idx);
                    MatrixNode *matrix_node_next = chance_node->access(state_copy.obs);
                    matrix_node_next->stats.depth = stats.depth + 1;
                    const typename Types::Prob prob = state_copy.prob;

                    auto alpha_beta_pair = double_oracle(
                        device,
                        state_copy,
                        model,
                        matrix_node_next,
                        min_val, max_val);

                    data.alpha_explored += alpha_beta_pair.first * prob;
                    data.beta_explored += alpha_beta_pair.second * prob;
                    expected_score += alpha_beta_pair.second * prob * col_strategy[best_i];

                    data.unexplored -= prob;
                    total_unexplored -= prob * col_strategy[best_i];
                    priority_scores[best_i] -= prob * col_strategy[best_i];

                    // assert(data.unexplored >= typename Types::Real{0});
                    // assert(total_unexplored >= typename Types::Real{0});

                    max_priority = typename Types::Q{0};
                    for (int i = 0; i < J.size(); ++i)
                    {
                        const typename Types::Real priority = priority_scores[i];
                        if (priority > max_priority)
                        {
                            col_idx = J[i];
                            max_priority = priority;
                            best_i = i;
                        }
                    }
                }

                if (expected_score >= alpha || (best_row_idx == -1 && fuzzy_equals(expected_score, alpha)))
                {
                    best_row_idx = row_idx;
                    alpha = expected_score;
                }
            }
            return {best_row_idx, alpha};
        }

        std::pair<int, typename Types::Real> best_response_col(
            Types::PRNG &device,
            const typename Types::State &state,
            typename Types::Model &model,
            MatrixNode *matrix_node,
            Types::Real alpha, typename Types::Real beta,
            Types::VectorReal &row_strategy)
        {
            MatrixStats &stats = matrix_node->stats;
            std::vector<int> &I = stats.I;
            std::vector<int> &J = stats.J;
            int best_col_idx = -1;

            for (int col_idx = 0; col_idx < state.col_actions.size(); ++col_idx)
            {
                bool col_idx_in_J = (std::find(J.begin(), J.end(), col_idx) != J.end());
                const typename Types::Action col_action = state.col_actions[col_idx];

                typename Types::Real max_priority{0}, expected_score{0}, total_unexplored{0};
                std::vector<typename Types::Real> priority_scores;
                int row_idx, best_i;
                for (int i = 0; i < I.size(); ++i)
                {
                    const int row_idx_temp = I[i];
                    Data &data = stats.data_matrix.get(row_idx_temp, col_idx);
                    expected_score += row_strategy[i] * data.alpha_explored;
                    typename Types::Real priority;
                    if (col_idx_in_J)
                    {
                        priority = typename Types::Q{0};
                    }
                    else
                    {
                        priority = row_strategy[i] * data.unexplored;
                    }
                    total_unexplored += priority;
                    priority_scores.push_back(priority);
                    if (priority > max_priority)
                    {
                        row_idx = row_idx_temp;
                        max_priority = priority;
                        best_i = i;
                    }
                }

                while (
                    fuzzy_greater(total_unexplored, typename Types::Real{Rational<>{0}}) &&
                    (typename Types::Real{expected_score + alpha * total_unexplored} <= beta))
                {
                    Data &data = stats.data_matrix.get(row_idx, col_idx);
                    const typename Types::Action row_action = state.row_actions[row_idx];
                    if (data.chance_actions.size() == 0)
                    {
                        state.get_chance_actions(row_action, col_action, data.chance_actions);
                    }

                    if (data.next_chance_idx >= data.chance_actions.size())
                    {
                        break;
                    }
                    typename Types::State state_copy = state;
                    state_copy.apply_actions(row_action, col_action, data.chance_actions[data.next_chance_idx++]);
                    ChanceNode *chance_node = matrix_node->access(row_idx, col_idx);
                    MatrixNode *matrix_node_next = chance_node->access(state_copy.obs);
                    matrix_node_next->stats.depth = stats.depth + 1;
                    const typename Types::Prob prob = state_copy.prob;

                    auto alpha_beta_pair = double_oracle(
                        device,
                        state_copy,
                        model,
                        matrix_node_next,
                        min_val, max_val);

                    data.alpha_explored += alpha_beta_pair.first * prob;
                    data.beta_explored += alpha_beta_pair.second * prob;
                    expected_score += alpha_beta_pair.first * prob * row_strategy[best_i];

                    data.unexplored -= prob;
                    total_unexplored -= prob * row_strategy[best_i];
                    priority_scores[best_i] -= prob * row_strategy[best_i];

                    // assert(data.unexplored >= typename Types::Real{0});
                    // assert(total_unexplored >= typename Types::Real{0});

                    max_priority = typename Types::Real{Rational<>{0}};
                    for (int i = 0; i < I.size(); ++i)
                    {
                        const typename Types::Real priority = priority_scores[i];
                        if (priority > max_priority)
                        {
                            row_idx = I[i];
                            max_priority = priority;
                            best_i = i;
                        }
                    }
                }

                if (expected_score <= beta || (best_col_idx == -1 && fuzzy_equals(expected_score, beta)))
                {
                    best_col_idx = col_idx;
                    beta = expected_score;
                }
            }
            return {best_col_idx, beta};
        }

    private:
        template <template <typename> typename Wrapper, typename T>
        inline bool fuzzy_equals(Wrapper<T> x, Wrapper<T> y)
        {
            if constexpr (std::is_same_v<T, mpq_class>)
            {
                mpq_ptr a = x.value.get_mpq_t();
                mpq_ptr b = y.value.get_mpq_t();
                mpq_canonicalize(a);
                mpq_canonicalize(b);
                bool answer = mpq_equal(a, b);
                return answer;
            }
            else
            {
                static const typename Types::Real epsilon{Rational{1, 1 << 24}};
                static const typename Types::Real neg_epsilon{Rational{-1, 1 << 24}};
                Wrapper<T> z{x - y};
                return neg_epsilon < z && z < epsilon;
            }
        }

        template <template <typename> typename Wrapper, typename T>
        inline bool fuzzy_greater(Wrapper<T> x, Wrapper<T> y)
        {
            if constexpr (std::is_same_v<T, mpq_class>)
            {
                return x > y;
            }
            else
            {
                static const typename Types::Real epsilon{Rational{1, 1 << 24}};
                bool a = x > y + epsilon;
                return a;
            }
        }

        inline bool try_solve_chance_node(
            Types::PRNG &device,
            const typename Types::State &state, // TODO const?
            Types::Model &model,
            MatrixNode *matrix_node,
            int row_idx, int col_idx)
        {
            MatrixStats &stats = matrix_node->stats;
            ChanceNode *chance_node = matrix_node->access(row_idx, col_idx);
            Data &data = stats.data_matrix.get(row_idx, col_idx);

            if (data.unexplored > typename Types::Prob{0})
            {
                auto row_action = state.row_actions[row_idx];
                auto col_action = state.col_actions[col_idx];

                auto &chance_actions = data.chance_actions;
                if (chance_actions.size() == 0)
                {
                    state.get_chance_actions(row_action, col_action, chance_actions); // TODO arg order
                }

                // go through all chance actions
                for (; data.next_chance_idx < chance_actions.size() && !terminate(device, data); ++data.next_chance_idx)
                {
                    if (chance_actions.size() <= data.next_chance_idx)
                    {
                        break;
                    }

                    const typename Types::Obs chance_action = chance_actions[data.next_chance_idx];
                    typename Types::State state_copy = state;
                    state_copy.apply_actions(row_action, col_action, chance_action);
                    MatrixNode *matrix_node_next = chance_node->access(state_copy.obs);
                    matrix_node_next->stats.depth = stats.depth + 1;
                    const typename Types::Prob prob = state_copy.prob;

                    auto alpha_beta = double_oracle(device, state_copy, model, matrix_node_next, min_val, max_val);

                    data.alpha_explored += alpha_beta.first * prob;
                    data.beta_explored += alpha_beta.second * prob;
                    data.unexplored -= prob;
                }
            }

            bool solved_exactly = (data.alpha_explored == data.beta_explored) && (data.unexplored == typename Types::Real{Rational<>{0}});
            return solved_exactly;
        };

        typename Types::Real row_alpha_beta(
            Types::State &state,
            typename Types::Model &model,
            MatrixNode *matrix_node,
            Types::Real alpha,
            Types::Real beta)
        {
            return max_val;
        }

        typename Types::Real col_alpha_beta(
            Types::State &state,
            typename Types::Model &model,
            MatrixNode *matrix_node,
            Types::Real alpha,
            Types::Real beta)
        {
            return min_val;
        }
        // Serialized AlphaBeta, TODO

        static bool dont_terminate(typename Types::PRNG &, const Data &)
        {
            return false;
            MatrixNode matrix_node;
        };

        static bool after_some_time(typename Types::PRNG &device, const Data &data)
        {
            const typename Types::Prob x{Rational<>{1, 4}};
            return (data.next_chance_idx > 1 &&
                    data.unexplored < x);
        }
    };
};