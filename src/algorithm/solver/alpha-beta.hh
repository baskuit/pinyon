#include <libsurskit/lrslib.hh>
#include <types/matrix.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

// #include <algorithm
#include <ranges>
#include <concepts>
#include <assert.h>

template <typename T>
concept DoubleOracleModelConcept = requires(T obj, typename T::Types::State &state) {
    typename T::Types;
    typename T::Types::ModelInput;
    typename T::Types::ModelOutput;
    // {
    //     obj.get_inference(
    //         std::reference_wrapper(typename T::Types::State{}),
    //         std::reference_wrapper(typename T::Types::ModelOutput{})
    //     )
    // } -> std::same_as<void>;
};

template <DoubleOracleModelConcept Model, template <class> class MNode = MatrixNode, template <class> class CNode = ChanceNode>
class AlphaBeta : public AbstractAlgorithm<Model>
{
private:
    int max_depth = -1;

public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = AlphaBeta::MatrixStats;
        using ChanceStats = AlphaBeta::ChanceStats;
    };

    struct Data
    {
        typename Types::Probability unexplored{typename Types::Rational{1}};
        typename Types::Real alpha_explored, beta_explored;
        int next_chance_idx = 0;
        std::vector<typename Types::Observation> chance_actions;
    };

    struct MatrixStats
    {
        DataMatrix<Data> data_matrix;
        typename Types::VectorReal row_solution, col_solution;

        size_t matrix_node_count = 1;
        size_t matrix_node_count_last = 0;
        unsigned int depth = 0;

        int row_pricipal_idx = 0, col_pricipal_idx = 0;
        std::vector<int> I, J;
    };

    static bool dont_terminate(typename Types::PRNG &, const Data &)
    {
        return false;
    };

    static bool after_some_time(typename Types::PRNG &device, const Data &data)
    {
        const typename Types::Probability x{Rational<>{1, 4}};
        return (data.next_chance_idx > 1 &&
                data.unexplored < x);
    }


    struct ChanceStats
    {
        size_t matrix_node_count = 0;
    };

    MNode<FullTraversal<Model>> *teacher;

    const typename Types::Real min_val{Rational<>{0}}; // don't need to use the Game values if you happen to know that State's
    const typename Types::Real max_val{Rational<>{1}};
    bool (*const terminate)(typename Types::PRNG &, const Data &) = &dont_terminate;

    // const typename Types::Real epsilon{Rational{1, 1 << 24}};

    AlphaBeta() {}

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : min_val(min_val), max_val(max_val) {}

    AlphaBeta(
        typename Types::Real min_val, typename Types::Real max_val,
        bool (*const terminate)(typename Types::PRNG &, const Data &)) : min_val(min_val), max_val(max_val), terminate{terminate} {}

    auto run(
        typename Types::PRNG &device,
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> &root)
    {
        return double_oracle(device, state, model, &root, min_val, max_val);
    }

    void run(
        size_t ms,
        typename Types::PRNG &device,
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> &root) {}

    std::pair<typename Types::Real, typename Types::Real>
    double_oracle(
        typename Types::PRNG &device,
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        MatrixStats &stats = matrix_node->stats;
        std::vector<int> &I = stats.I;
        std::vector<int> &J = stats.J;

        state.get_actions();
        matrix_node->expand(state);

        if (state.is_terminal)
        {
            matrix_node->set_terminal();
            return {state.payoff.get_row_value(), state.payoff.get_row_value()};
        }
        if (max_depth > 0 && stats.depth >= max_depth)
        {
            matrix_node->set_terminal();
            typename Types::ModelOutput inference;
            model.get_inference(state, inference);
            return {inference.value.get_row_value(), inference.value.get_row_value()};
        }

        // here is the only place we can use prior solve info

        I.push_back(stats.row_pricipal_idx);
        J.push_back(stats.col_pricipal_idx);
        bool solved_exactly = true;

        const size_t rows = state.row_actions.size();
        const size_t cols = state.col_actions.size();

        stats.data_matrix.fill(rows, cols);

        bool smaller_bounds = false;
        bool new_action = true;
        typename Types::VectorReal row_strategy, col_strategy;

        int latest_row_idx = 0;
        int latest_col_idx = 0;

        while (!fuzzy_equals(alpha, beta) && (smaller_bounds || new_action))
        {

            // get entry values/bounds for newly expanded sub game, use last added actions

            for (const int row_idx : I)
            {
                Data &data = stats.data_matrix.get(row_idx, latest_col_idx);
                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, latest_col_idx);
                solved_exactly &= try_solve_chance_node(device, state, model, matrix_node, row_idx, latest_col_idx, min_val, max_val);
            }

            for (const int col_idx : J)
            {
                Data &data = stats.data_matrix.get(latest_row_idx, col_idx);
                CNode<AlphaBeta> *chance_node = matrix_node->access(latest_row_idx, col_idx);
                solved_exactly &= try_solve_chance_node(device, state, model, matrix_node, latest_row_idx, col_idx, min_val, max_val);
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

        stats.row_solution = row_strategy;
        stats.col_solution = col_strategy;
        return {alpha, beta};
    }

    std::pair<int, typename Types::Real>
    best_response_row(
        typename Types::PRNG &device,
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha, typename Types::Real beta,
        typename Types::VectorReal &col_strategy)
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
                    priority = typename Types::Rational{0};
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
                    state.get_chance_actions(data.chance_actions, row_action, col_action);
                }

                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, data.chance_actions[data.next_chance_idx++]);
                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                auto alpha_beta_pair = double_oracle(
                    device,
                    state_copy,
                    model,
                    matrix_node_next,
                    min_val, max_val);

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                expected_score += alpha_beta_pair.second * state_copy.prob * col_strategy[best_i];

                data.unexplored -= state_copy.prob;
                total_unexplored -= state_copy.prob * col_strategy[best_i];
                priority_scores[best_i] -= state_copy.prob * col_strategy[best_i];

                max_priority = typename Types::Rational{0};
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

            if (expected_score >= alpha)
            {
                best_row_idx = row_idx;
                alpha = expected_score;
            }
        }
        return {best_row_idx, alpha};
    }

    std::pair<int, typename Types::Real> best_response_col(
        typename Types::PRNG &device,
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha, typename Types::Real beta,
        typename Types::VectorReal &row_strategy)
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
                    priority = typename Types::Rational{0};
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
                (max_priority > typename Types::Real{Rational<>{0}}) &&
                (typename Types::Real{expected_score + alpha * total_unexplored} <= beta))
            {
                Data &data = stats.data_matrix.get(row_idx, col_idx);
                const typename Types::Action row_action = state.row_actions[row_idx];
                if (data.chance_actions.size() == 0)
                {
                    state.get_chance_actions(data.chance_actions, row_action, col_action);
                }

                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, data.chance_actions[data.next_chance_idx++]);
                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                auto alpha_beta_pair = double_oracle(
                    device,
                    state_copy,
                    model,
                    matrix_node_next,
                    min_val, max_val);

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                expected_score += alpha_beta_pair.first * state_copy.prob * row_strategy[best_i];

                data.unexplored -= state_copy.prob;
                total_unexplored -= state_copy.prob * row_strategy[best_i];
                priority_scores[best_i] -= state_copy.prob * row_strategy[best_i];

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

            if (expected_score <= beta)
            {
                best_col_idx = col_idx;
                beta = expected_score;
            }
        }
        return {best_col_idx, beta};
    }

private:
    template <typename T>
    bool fuzzy_equals(T x, T y)
    {
        mpq_ptr a = x.value.get_mpq_t();
        mpq_ptr b = y.value.get_mpq_t();
        // mpq_canonicalize(a);
        // mpq_canonicalize(b);
        bool answer = mpq_equal(a, b);
        return answer; // TODO
    }

    inline bool try_solve_chance_node(
        typename Types::PRNG &device,
        typename Types::State &state, // TODO const?
        typename Types::Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        int row_idx, int col_idx,
        typename Types::Real alpha, typename Types::Real beta)
    {
        MatrixStats &stats = matrix_node->stats;
        CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
        Data &data = stats.data_matrix.get(row_idx, col_idx);

        if (data.unexplored > typename Types::Probability{0})
        {
            auto row_action = state.row_actions[row_idx];
            auto col_action = state.col_actions[col_idx];

            auto &chance_actions = data.chance_actions;
            if (chance_actions.size() == 0)
            {
                state.get_chance_actions(chance_actions, row_action, col_action); // TODO arg order
            }

            // go through all chance actions
            for (; data.next_chance_idx < chance_actions.size() && !terminate(device, data); ++data.next_chance_idx)
            {
                const typename Types::Observation chance_action = chance_actions[data.next_chance_idx];
                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, chance_action);
                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                auto alpha_beta = double_oracle(device, state_copy, model, matrix_node_next, alpha, beta);

                data.alpha_explored += alpha_beta.first * state_copy.prob;
                data.beta_explored += alpha_beta.second * state_copy.prob;
                data.unexplored -= state_copy.prob;
            }
        }

        bool solved_exactly = (data.alpha_explored == data.beta_explored) && (data.unexplored == typename Types::Real{Rational<>{0}});
        return solved_exactly;
    };

    typename Types::Real row_alpha_beta(
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        return max_val;
    }

    typename Types::Real col_alpha_beta(
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        return min_val;
    }
    // Serialized AlphaBeta, TODO
};