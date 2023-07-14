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

    int max_depth = -1;
    const typename Types::Real epsilon{Rational{1, 1 << 24}};

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : min_val(min_val), max_val(max_val) {}

    void run(
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *root,
        int max_depth = -1)
    {
        double_oracle(state, model, root, min_val, max_val, max_depth);
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
            return {state.payoff.get_row_value(), state.payoff.get_col_value()};
        }
        if (max_depth > 0 && stats.depth >= max_depth)
        {
            matrix_node->set_terminal();
            typename Types::ModelOutput inference;
            model.get_inference(state, inference);
            return {inference.value.get_row_value(), inference.value.get_col_value()};
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
                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, latest_col_idx);
                solved_exactly &= try_solve_chance_node(state, model, chance_node, row_idx, latest_col_idx, stats.data_matrix.get(row_idx, latest_col_idx), stats.depth);
            }
            for (const int col_idx : J)
            {
                CNode<AlphaBeta> *chance_node = matrix_node->access(latest_row_idx, col_idx);
                solved_exactly &= try_solve_chance_node(state, model, chance_node, latest_row_idx, col_idx, stats.data_matrix.get(latest_row_idx, col_idx), stats.depth);
            }

            // solve newly expanded and explored game
            typename Types::VectorReal row_strategy, col_strategy;

            int entry_idx = 0;
            if (solved_exactly)
            {
                typename Types::MatrixValue matrix{rows, cols};
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
                typename Types::MatrixValue alpha_matrix{rows, cols}, beta_matrix{rows, cols};
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
            auto iv = best_response_row(state, model, matrix_node, alpha, col_strategy, I, J);
            auto jv = best_response_col(state, model, matrix_node, beta, row_strategy, I, J); // use p aka alpha_matrix

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
        const typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha,
        typename Types::VectorReal &col_strategy,
        std::vector<int> &I,
        std::vector<int> &J)
    {
        MatrixStats &stats = matrix_node->stats;
        int best_row_idx = 0;
        for (int row_idx = 0; row_idx < state.row_actions.size(); ++row_idx)
        {

            const typename Types::Action row_action = state.row_actions[row_idx];

            typename Types::Real max = typename Types::Rational{0};
            std::vector<typename Types::Real> importance_weight;
            int col_idx = 0;
            int best_i; // replace with ptr to Real?

            for (int i = 0; i < J.size(); ++i)
            {
                const int col_idx__ = J[i];
                const typename Types::Real x = col_strategy[col_idx__] * stats.data_matrix.get(row_idx, col_idx__).unexplored;
                importance_weight.push_back(x);
                if (x > max)
                {
                    col_idx = col_idx__;
                    max = x;
                    best_i = i;
                }
            }

            while (true)
            {
                Data &data = stats.data_matrix.get(row_idx, col_idx);

                if (max == 0)
                {
                    break;
                    // this means we have solved every matrix_node_child in this row
                }

                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);

                // get AND increment
                int chance_idx = data.next_chance_idx++;

                const typename Types::Action col_action = state.col_actions[col_idx];
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
                importance_weight[best_i] -= state_copy.prob * col_strategy[col_idx];

                max = typename Types::Rational{0};
                for (int i = 0; i < J.size(); ++i)
                {
                    const typename Types::Real x = importance_weight[i];
                    if (x > max)
                    {
                        col_idx = J[i];
                        max = x;
                    }
                }
            }
        }
        return {best_row_idx, alpha};
    }

    std::pair<int, typename Types::Real> best_response_col(
        const typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real beta,
        typename Types::VectorReal &row_strategy,
        std::vector<int> &I,
        std::vector<int> &J)
    {
        MatrixStats &stats = matrix_node->stats;

        for (int col_idx = 0; col_idx < state.col_actions.size(); ++col_idx)
        {

            const typename Types::Action col_action = state.col_actions[col_idx];

            std::vector<typename Types::Real> importance_weight;
            for (const int row_idx : I)
            {
                importance_weight.push_back(
                    static_cast<typename Types::Real>(row_strategy[row_idx] * stats.data_matrix.get(row_idx, col_idx).unexplored));
            }

            while (true)
            {

                int row_idx = 0;

                Data &data = stats.data_matrix.get(row_idx, col_idx);

                const typename Types::Real max_importance_weight;
                if (max_importance_weight == 0)
                {
                    break;
                }

                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);

                int chance_idx = data.next_chance_idx++;

                const typename Types::Action row_action = state.row_actions[row_idx];
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
                    alpha_next, beta_next,
                    0);

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                data.unexplored -= state_copy.prob;
            }
        }
        return {0, beta};
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
        int depth)
    {

        if (data.unexplored > 0)
        {

            // typename Types::Action row_action, col_action;
            auto row_action = state.row_actions[row_idx];
            auto col_action = state.col_actions[col_idx];

            auto chance_actions = data.chance_actions;
            if (chance_actions.size() == 0)
            {
                state.get_chance_actions(chance_actions, row_action, col_action); // TODO arg order
            }

            for (; data.next_chance_idx < chance_actions.size(); ++data.next_chance_idx)
            {
                const typename Types::Observation chance_action = data.chance_actions[data.next_chance_idx];
                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, chance_action);

                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                typename Types::Real alpha_next = data.alpha_explored + min_val * data.unexplored;
                typename Types::Real beta_next = data.beta_explored + max_val * data.unexplored;

                auto alpha_beta = double_oracle(state_copy, model, matrix_node_next, alpha_next, beta_next, depth + 1);
                data.unexplored -= state.prob;
                data.alpha_explored += alpha_beta.first * state_copy.prob;
                data.beta_explored += alpha_beta.second * state_copy.prob;
            }
        }

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