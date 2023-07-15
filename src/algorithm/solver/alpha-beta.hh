#include <libsurskit/lrslib.hh>
#include <types/matrix.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

// #include <algorithm
#include <ranges>
#include <concepts>

/*

An implementation of Simultaneous Move Alpha Beta

See "Using Double-Oracle Method and Serialized Alpha-Beta Search
for Pruning in Simultaneous Move Games>

*/

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
        typename Types::Probability unexplored = typename Types::Rational{1};

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

    struct ChanceStats
    {
        // typename Types::Probability explored{Rational{0}};
        size_t matrix_node_count = 0;
    };

    const typename Types::Real min_val{0}; // don't need to use the Game values if you happen to know that State's
    const typename Types::Real max_val{1};

    const typename Types::Real epsilon{Rational{1, 1 << 24}};

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : min_val(min_val), max_val(max_val) {}

    auto run(
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *root,
        int max_depth = -1)
    {
        return double_oracle(state, model, root, min_val, max_val, max_depth);
    }

    std::pair<typename Types::Real, typename Types::Real>
    double_oracle(
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::Real beta,
        int max_depth)
    {

        MatrixStats &stats = matrix_node->stats;

        typename Types::Real alpha_new;
        typename Types::Real beta_new;

        std::vector<int> I{}, J{};

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

        I.push_back(stats.row_pricipal_idx);
        J.push_back(stats.col_pricipal_idx);
        bool solved_exactly = true;

        const size_t rows = state.row_actions.size();
        const size_t cols = state.col_actions.size();

        stats.data_matrix.fill(rows, cols);

        while (!fuzzy_equals(alpha, beta) && true)
        {

            // get entry values/bounds for newly expanded sub game, use last added actions
            const int latest_row_idx = I.back();
            const int latest_col_idx = J.back();

            const typename Types::Action latest_row_action = state.row_actions[latest_row_idx];
            const typename Types::Action latest_col_action = state.col_actions[latest_col_idx];

            for (const int row_idx : I)
            {
                Data &data = stats.data_matrix.get(row_idx, latest_col_idx);
                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, latest_col_idx);
                solved_exactly &= try_solve_chance_node(state, model, chance_node, row_idx, latest_col_idx, data, max_depth, alpha, beta);
            }
            for (const int col_idx : J)
            {
                if (col_idx == latest_col_idx)
                {
                    continue; // already done above
                }
                Data &data = stats.data_matrix.get(latest_row_idx, col_idx);
                CNode<AlphaBeta> *chance_node = matrix_node->access(latest_row_idx, col_idx);
                solved_exactly &= try_solve_chance_node(state, model, chance_node, latest_row_idx, col_idx, data, max_depth, alpha, beta);
            }

            // solve newly expanded and explored game
            typename Types::VectorReal row_strategy, col_strategy;

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
                alpha_new = beta_new = LRSNash::solve(matrix, row_strategy, col_strategy).first;
            }
            else
            {
                typename Types::MatrixValue alpha_matrix{I.size(), J.size()}, beta_matrix{I.size(), J.size()};
                for (auto row_idx : I)
                {
                    for (auto col_idx : J)
                    {
                        const Data &data = stats.data_matrix.get(row_idx, col_idx);
                        alpha_matrix[entry_idx] = data.alpha_explored;
                        beta_matrix[entry_idx] = data.beta_explored;
                        ++entry_idx;
                    }
                }
                typename Types::VectorReal temp;
                alpha_new = LRSNash::solve(alpha_matrix, temp, col_strategy).first;
                temp.clear();
                beta_new = LRSNash::solve(beta_matrix, row_strategy, temp).first;
            }

            // best response step
            auto iv = best_response_row(state, model, matrix_node, alpha, col_strategy, I, J, max_depth);
            auto jv = best_response_col(state, model, matrix_node, beta, row_strategy, I, J, max_depth); // use p aka alpha_matrix

            stats.row_solution = row_strategy;
            stats.col_solution = col_strategy;

            if (iv.first == -1)
            {
                return {min_val, min_val};
            }
            if (jv.first == -1)
            {
                return {max_val, max_val};
            }

            alpha = std::max(alpha, jv.second);
            beta = std::min(beta, iv.second);

            if (std::find(I.begin(), I.end(), iv.first) == I.end())
            {
                I.push_back(iv.first);
            }
            if (std::find(J.begin(), J.end(), jv.first) == J.end())
            {
                J.push_back(jv.first);
            }
        }
        return {alpha, beta};
    }

    std::pair<int, typename Types::Real> best_response_row(
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::VectorReal &col_strategy,
        std::vector<int> &I,
        std::vector<int> &J,
        int max_depth)
    {
        MatrixStats &stats = matrix_node->stats;
        int best_row_idx = -1;
        for (int row_idx = 0; row_idx < state.row_actions.size(); ++row_idx)
        {

            const typename Types::Action row_action = state.row_actions[row_idx];

            typename Types::Real max = typename Types::Rational{0};
            std::vector<typename Types::Real> importance_weight;
            int col_idx = 0;
            int best_i = 0; // replace with ptr to Real?

            for (int i = 0; i < J.size(); ++i)
            {
                const int col_idx_temp = J[i];
                const typename Types::Real x = col_strategy[i] * stats.data_matrix.get(row_idx, col_idx_temp).unexplored;
                importance_weight.push_back(x);
                if (x > max)
                {
                    col_idx = col_idx_temp;
                    max = x;
                    best_i = i;
                }
            }

            while (max > typename Types::Rational{0})
            {
                Data &data = stats.data_matrix.get(row_idx, col_idx);

                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);

                int chance_idx = data.next_chance_idx++;

                const typename Types::Action col_action = state.col_actions[col_idx];
                if (data.chance_actions.size() == 0)
                {
                    state.get_chance_actions(data.chance_actions, row_action, col_action);
                }
                const typename Types::Observation chance_action = data.chance_actions[chance_idx];
                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, chance_action);
                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                const typename Types::Real alpha_next = data.alpha_explored + min_val * data.unexplored; // TODO is this correct?
                const typename Types::Real beta_next = data.beta_explored + max_val * data.unexplored;

                auto alpha_beta_pair = double_oracle(
                    state_copy,
                    model,
                    matrix_node_next,
                    alpha_next, beta_next, max_depth);

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                data.unexplored -= state_copy.prob;
                typename Types::Real z = state_copy.prob * col_strategy[best_i];
                importance_weight[best_i] -= z;

                max = typename Types::Rational{0};
                for (int i = 0; i < J.size(); ++i)
                {
                    const typename Types::Real x = importance_weight[i];
                    if (x > max)
                    {
                        col_idx = J[i];
                        max = x;
                        best_i = i;
                    }
                }
            }
            // done solving

            typename Types::Real expected_score = typename Types::Rational{0};
            for (int i = 0; i < J.size(); ++i)
            {
                const int col_idx = J[i];
                Data &data = stats.data_matrix.get(row_idx, col_idx);
                expected_score += col_strategy[i] * data.alpha_explored;
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
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real beta,
        typename Types::VectorReal &row_strategy,
        std::vector<int> &I,
        std::vector<int> &J,
        int max_depth)
    {
        MatrixStats &stats = matrix_node->stats;
        int best_col_idx = -1;
        for (int col_idx = 0; col_idx < state.col_actions.size(); ++col_idx)
        {

            const typename Types::Action col_action = state.col_actions[col_idx];

            typename Types::Real max = typename Types::Rational{0};
            std::vector<typename Types::Real> importance_weight;
            int row_idx = 0;
            int best_i = 0;
            for (int i = 0; i < I.size(); ++i)
            {
                const int row_idx_temp = I[i];
                const typename Types::Real x = row_strategy[i] * stats.data_matrix.get(row_idx_temp, col_idx).unexplored;
                importance_weight.push_back(x);
                if (x > max)
                {
                    row_idx = row_idx_temp;
                    max = x;
                    best_i = i;
                }
            }

            while (max > typename Types::Rational{0})
            {
                Data &data = stats.data_matrix.get(row_idx, col_idx);

                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);

                int chance_idx = data.next_chance_idx++;

                const typename Types::Action row_action = state.row_actions[row_idx];
                if (data.chance_actions.size() == 0)
                {
                    state.get_chance_actions(data.chance_actions, row_action, col_action);
                }
                const typename Types::Observation chance_action = data.chance_actions[chance_idx];
                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, chance_action);
                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                const typename Types::Real alpha_next = data.alpha_explored + min_val * data.unexplored;
                const typename Types::Real beta_next = data.beta_explored + max_val * data.unexplored;

                auto alpha_beta_pair = double_oracle(
                    state_copy,
                    model,
                    matrix_node_next,
                    alpha_next, beta_next, max_depth);

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                data.unexplored -= state_copy.prob;
                typename Types::Real z = state_copy.prob * row_strategy[best_i];
                importance_weight[best_i] -= z;

                max = typename Types::Rational{0};
                for (int i = 0; i < I.size(); ++i)
                {
                    const typename Types::Real x = importance_weight[i];
                    if (x > max)
                    {
                        row_idx = I[i];
                        max = x;
                        best_i = i;
                    }
                }
            }

            typename Types::Real expected_score = typename Types::Rational{0};
            for (int i = 0; i < I.size(); ++i)
            {
                const int row_idx = I[i];
                Data &data = stats.data_matrix.get(row_idx, col_idx);
                expected_score += row_strategy[i] * data.beta_explored;
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
        // return epsilon > std::abs((x - y).unwrap());
        mpq_ptr a = x.value.get_mpq_t();
        mpq_ptr b = y.value.get_mpq_t();
        mpq_canonicalize(a);
        mpq_canonicalize(b);
        bool answer = mpq_equal(a, b);
        return answer; // TODO
    }

    inline bool try_solve_chance_node(
        typename Types::State &state, // TODO const?
        typename Types::Model &model,
        CNode<AlphaBeta> *chance_node,
        int row_idx,
        int col_idx,
        Data &data,
        int depth,
        typename Types::Real alpha,
        typename Types::Real beta)
    {

        if (data.unexplored > typename Types::Rational{0})
        {

            // typename Types::Action row_action, col_action;
            auto row_action = state.row_actions[row_idx];
            auto col_action = state.col_actions[col_idx];

            auto &chance_actions = data.chance_actions;
            if (chance_actions.size() == 0)
            {
                state.get_chance_actions(chance_actions, row_action, col_action); // TODO arg order
            }

            for (; data.next_chance_idx < chance_actions.size(); ++data.next_chance_idx)
            {
                const typename Types::Observation chance_action = chance_actions[data.next_chance_idx];
                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, chance_action);

                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                auto alpha_beta = double_oracle(state_copy, model, matrix_node_next, alpha, beta, depth);
                data.unexplored -= state.prob;
                data.alpha_explored += alpha_beta.first * state_copy.prob;
                data.beta_explored += alpha_beta.second * state_copy.prob;
            }
        }
        mpq_canonicalize(data.alpha_explored.value.get_mpq_t());
        mpq_canonicalize(data.beta_explored.value.get_mpq_t());

        bool solved_exactly = (data.alpha_explored == data.beta_explored);

        // if (data.explored != typename Types::Rational{1})
        // {
        //     exit(1);
        // }

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