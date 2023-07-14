#include <libsurskit/gambit.hh>
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
concept DoubleOracleModelConcept = requires(T obj) {
    typename T::Types;
    typename T::Types::ModelInput;
    typename T::Types::ModelOutput;
    // {
    //     obj.get_inference(typename T::Types::State{}, typename T::Types::ModelOutput{})
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
        typename Types::Probability explored = typename Types::Rational{0};
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

        // 6: initialize restricted action sets I and J with a first action in stage s

        // TODO first action using info from last expansion, if possible

        I.push_back(stats.row_pricipal_idx);
        J.push_back(stats.col_pricipal_idx);
        bool solved_exactly = true;

        const size_t rows = state.row_actions.size();
        const size_t cols = state.col_actions.size();

        stats.data_matrix.fill(rows, cols);
        // Note: this implementation does not use serialized alpha beta
        // Just seems like too much tree traversal?
        // 9: repeat, 23: until α = β

        while (!fuzzy_equals(alpha, beta) && true)
        {

            const int latest_row_idx = I.back();
            const int latest_col_idx = J.back();

            const typename Types::Action latest_row_action = state.row_actions[latest_row_idx];
            const typename Types::Action latest_col_action = state.col_actions[latest_col_idx];

            // 10: for i ∈ I, j ∈ J do
            for (const int row_idx : I)
            {
                CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, latest_col_idx);
                solved_exactly &= foo(state, model, chance_node, row_idx, latest_col_idx, stats.data_matrix.get(row_idx, latest_col_idx), stats.depth);
            }
            for (const int col_idx : J)
            {
                CNode<AlphaBeta> *chance_node = matrix_node->access(latest_row_idx, col_idx);
                solved_exactly &= foo(state, model, chance_node, latest_row_idx, col_idx, stats.data_matrix.get(latest_row_idx, col_idx), stats.depth);
            }

            typename Types::VectorReal row_strategy, col_strategy;

            if (solved_exactly)
            {
                typename Types::MatrixValue matrix;

                alpha_new = solve_submatrix(matrix, matrix_node, row_strategy, col_strategy, I, J);
                beta_new = alpha_new;
            }
            else
            {
                typename Types::MatrixValue alpha_matrix{rows, cols}, beta_matrix{rows, cols};

                int entry_idx = 0;
                for (const Data &data : stats.data_matrix)
                {
                    alpha_matrix[entry_idx] = data.alpha_explored;
                    beta_matrix[entry_idx] = data.beta_explored;
                }

                typename Types::VectorReal temp;
                alpha_new = solve_submatrix(alpha_matrix, matrix_node, temp, col_strategy, I, J);
                beta_new = solve_submatrix(beta_matrix, matrix_node, row_strategy, temp, I, J);
            }

            // 15: hi0, vMaxi ← BRMax (s, α, y)
            auto iv = best_response_row(state, model, matrix_node, alpha, col_strategy, I, J);
            // 16: h j0 , vMini ← BRMin(s, β, x)
            auto jv = best_response_col(state, model, matrix_node, beta, row_strategy, I, J);

            stats.row_solution = row_strategy;
            stats.col_solution = col_strategy;

            // 17 - 20
            if (iv.first == -1)
            {
                return {min_val, max_val};
            }
            if (jv.first == -1)
            {
                return {min_val, max_val};
            }
            // 21: α ← max(α, vMin); β ← min(β, v Max )
            alpha = std::max(alpha, jv.second);
            beta = std::min(beta, iv.second);

            // 22: I ← I ∪ {i0}; J ← J ∪ { j0 }
            if (std::find(I.begin(), I.end(), iv.first) == I.end())
            {
                I.push_back(iv.first);
            }
            if (std::find(J.begin(), J.end(), jv.first) == J.end())
            {
                J.push_back(jv.first);
            }
        }
        return {alpha_new, beta_new};
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

        for (int row_idx = 0; row_idx < state.row_actions.size(); ++row_idx)
        {

            const typename Types::Action row_action = state.row_actions[row_idx];

            // get importance weight for each col action with support
            std::vector<typename Types::Real> importance_weight;
            // importance_weight.reserve(J.size());
            for (const int col_idx : J)
            {
                importance_weight.push_back(
                    static_cast<typename Types::Real>(col_strategy[col_idx] * stats.data_matrix.get(row_idx, col_idx).unexplored));
            }

            while (true)
            {

                // get col_idx with largest importance
                int col_idx = 0;

                Data &data = stats.data_matrix.get(row_idx, col_idx);

                const typename Types::Real max_importance_weight;
                if (max_importance_weight == 0)
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
                data.explored += state_copy.prob;
                data.unexplored -= state_copy.prob;
            }
        }
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
                                           alpha_next, beta_next) *
                                       state_copy.prob;

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                data.explored += state_copy.prob;
                data.unexplored -= state_copy.prob;
            }
        }
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

    typename Types::Real solve_submatrix(
        typename Types::MatrixValue &submatrix,
        MNode<AlphaBeta> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy,
        std::vector<int> &I, std::vector<int> &J)
    {
        // define submatrix
        submatrix.fill(I.size(), J.size());
        row_strategy.fill(submatrix.rows);
        col_strategy.fill(submatrix.cols);
        int entry_idx = 0;
        for (const int row_idx : I)
        {
            for (const int col_idx : J)
            {
                submatrix[entry_idx++] = typename Types::Value{matrix_node->stats.p.get(row_idx, col_idx)};
                // assert(matrix_node->stats.p.get(row_idx, col_idx) == matrix_node->stats.o.get(row_idx, col_idx));
                // we can use either p or q here since the substage is solved
            }
        }

        LRSNash::solve(submatrix, row_strategy, col_strategy);

        typename Types::Real value = typename Types::Rational{0};
        for (int row_idx = 0; row_idx < submatrix.rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < submatrix.cols; ++col_idx)
            {
                value += submatrix.get(row_idx, col_idx).get_row_value() * row_strategy[row_idx] * col_strategy[col_idx];
            }
        }
        return value;
    }

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

    inline bool foo(
        const typename Types::State &state,
        typename Types::Model &model,
        CNode<AlphaBeta> *chance_node,
        int row_idx,
        int col_idx,
        Data &data,
        int depth) const
    {

        if (data.explored < 1)
        {

            const typename Types::Action row_action, col_action;
            row_action = state.row_actions[row_idx];
            col_action = state.col_actions[col_idx];

            auto chance_actions = data.chance_actions;
            if (chance_actions.size() == 0)
            {
                state.get_chance_actions(row_action, col_action, chance_actions);
            }

            for (; data.next_chance_idx < chance_actions.size(); ++data.next_chance_idx)
            {
                const typename Types::Observation chance_action = data.chance_actions[data.next_chance_idx];
                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, chance_action);

                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);

                typename Types::Real alpha_next = data.alpha_explored + min_val * data.unexplored;
                typename Types::Real beta_next = data.beta_explored + max_val * data.unexplored;

                auto alpha_beta = double_oracle(state_copy, model, matrix_node_next, alpha_next, beta_next, depth + 1) * state_copy.transition.prob;
                data.explored += state.prob;
                data.unexplored -= state.prob;
                data.alpha_explored += alpha_beta.first;
                data.beta_explored += alpha_beta.second;
            }
        }

        bool solved_exactly = (data.alpha_explored == data.beta_explored);

        assert(data.explored == 1);

        return solved_exactly;
    };
};