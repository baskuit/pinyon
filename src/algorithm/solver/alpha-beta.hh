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
        bool solved = false;
        typename Types::Probability unexplored = typename Types::Rational{1};

        typename Types::Real alpha_explored, beta_explored;
        int next_chance_idx = 0;
        std::vector<typename Types::Observation> chance_actions;
        std::vector<double> explored_vec{};
        std::vector<int> chance_idx_vec{};
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

    static bool dont_terminate(typename Types::PRNG &, MatrixStats &)
    {
        return false;
    };

    bool (*const terminate)(typename Types::PRNG &, MatrixStats &) = dont_terminate;

    struct ChanceStats
    {
        size_t matrix_node_count = 0;
    };

    MNode<FullTraversal<Model>> *teacher;

    const typename Types::Real min_val{0}; // don't need to use the Game values if you happen to know that State's
    const typename Types::Real max_val{1};

    // const typename Types::Real epsilon{Rational{1, 1 << 24}};

    AlphaBeta(typename Types::Real min_val, typename Types::Real max_val) : min_val(min_val), max_val(max_val) {}

    auto run( // TODO depth param
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

        assert(teacher->stats.payoff.row_value >= alpha && teacher->stats.payoff.row_value <= beta);

        MatrixStats &stats = matrix_node->stats;

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

        // here is the only place we can use prior solve info

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
                solved_exactly &= try_solve_chance_node(device, state, model, matrix_node, row_idx, latest_col_idx, min_val, max_val);
            }
            for (const int col_idx : J)
            {
                if (col_idx == latest_col_idx)
                {
                    continue; // already done above
                }
                Data &data = stats.data_matrix.get(latest_row_idx, col_idx);
                CNode<AlphaBeta> *chance_node = matrix_node->access(latest_row_idx, col_idx);
                solved_exactly &= try_solve_chance_node(device, state, model, matrix_node, latest_row_idx, col_idx, min_val, max_val);
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
                // std::cout << "alpha_matrix" << std::endl;
                // alpha_matrix.print();
                // std::cout << "beta_matrix" << std::endl;
                // beta_matrix.print();
                // std::cout << "correct:" <<std::endl;
                // teacher->stats.nash_payoff_matrix.print();
            }

            std::pair<int, typename Types::Real>
                iv = best_response_row(
                    device,
                    state,
                    model,
                    matrix_node,
                    alpha, max_val,
                    col_strategy,
                    I, J);

            std::pair<int, typename Types::Real>
                jv = best_response_col(
                    device,
                    state,
                    model,
                    matrix_node,
                    min_val, beta,
                    row_strategy,
                    I, J);

            if (iv.first == -1)
            {
                stats.row_solution = row_strategy;
                stats.col_solution = col_strategy;
                assert(teacher->stats.payoff.row_value == min_val);
                return {min_val, min_val};
            }
            if (jv.first == -1)
            {
                stats.row_solution = row_strategy;
                stats.col_solution = col_strategy;
                assert(teacher->stats.payoff.row_value == max_val);
                return {max_val, max_val};
            }

            bool no_new_actions = true;
            if (std::find(I.begin(), I.end(), iv.first) == I.end())
            {
                no_new_actions = false;
                I.push_back(iv.first);
            }
            if (std::find(J.begin(), J.end(), jv.first) == J.end())
            {
                no_new_actions = false;
                J.push_back(jv.first);
            }

            bool smaller_bounds = false;
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

            if (no_new_actions && !smaller_bounds)
            {
                stats.row_solution = row_strategy;
                stats.col_solution = col_strategy;
                break;
            }
        }

        assert(teacher->stats.payoff.row_value <= beta);
        assert(teacher->stats.payoff.row_value >= alpha);

        return {alpha, beta};
    }

    std::pair<int, typename Types::Real> best_response_row(
        typename Types::PRNG &device,
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha, typename Types::Real beta,
        typename Types::VectorReal &col_strategy,
        std::vector<int> &I, std::vector<int> &J)
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
                Data &data = stats.data_matrix.get(row_idx, col_idx_temp);
                typename Types::Real x;
                if (data.solved)
                {
                    x = Rational<>{0};
                }
                else
                {
                    x = col_strategy[i] * data.unexplored;
                }
                importance_weight.push_back(x);
                if (x > max)
                {
                    col_idx = col_idx_temp;
                    max = x;
                    best_i = i;
                }
            }

            mpq_canonicalize(max.value.get_mpq_t());

            while (max > typename Types::Rational{0})
            {
                Data &data = stats.data_matrix.get(row_idx, col_idx);
                if (data.solved)
                {
                    break;
                }
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
                auto old_teacher = teacher;
                teacher = teacher->access(row_idx, col_idx)->access(state_copy.obs);

                auto alpha_beta_pair = double_oracle(
                    device,
                    state_copy,
                    model,
                    matrix_node_next,
                    min_val, max_val);

                teacher = old_teacher;

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                data.unexplored -= state_copy.prob;
                data.explored_vec.push_back(state_copy.prob.value.get_d());
                data.chance_idx_vec.push_back(chance_idx);

                assert(data.unexplored >= static_cast<typename Types::Probability>(Rational<>{0}));
                if (chance_idx == state_copy.transitions - 1)
                {
                    assert(data.unexplored == static_cast<typename Types::Probability>(Rational<>{0}));
                }

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

                // if (data.unexplored > Rational<>{0})
                // {
                //     std::cout << '!' << std::endl;
                // }

                const typename Types::Real alpha_next = data.beta_explored + beta * data.unexplored; // TODO is this correct?
                const typename Types::Real w = col_strategy[i] * alpha_next;
                expected_score += w;
            }

            mpq_canonicalize(expected_score.value.get_mpq_t());
            if (expected_score >= alpha)
            {
                best_row_idx = row_idx;
                alpha = expected_score;
            }
        }
        mpq_canonicalize(alpha.value.get_mpq_t());
        return {best_row_idx, alpha};
    }

    std::pair<int, typename Types::Real> best_response_col(
        typename Types::PRNG &device,
        typename Types::State &state,
        Model &model,
        MNode<AlphaBeta> *matrix_node,
        typename Types::Real alpha, typename Types::Real beta,
        typename Types::VectorReal &row_strategy,
        std::vector<int> &I, std::vector<int> &J)
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
                Data &data = stats.data_matrix.get(row_idx_temp, col_idx);
                typename Types::Real x;
                if (data.solved)
                {
                    x = typename Types::Rational{0};
                }
                else
                {
                    x = row_strategy[i] * data.unexplored;
                }
                importance_weight.push_back(x);
                if (x > max)
                {
                    row_idx = row_idx_temp;
                    max = x;
                    best_i = i;
                }
            }

            mpq_canonicalize(max.value.get_mpq_t());

            while (max > typename Types::Rational{0})
            {
                Data &data = stats.data_matrix.get(row_idx, col_idx);
                if (data.solved)
                {
                    break;
                }

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
                auto old_teacher = teacher;
                teacher = teacher->access(row_idx, col_idx)->access(state_copy.obs);

                auto alpha_beta_pair = double_oracle(
                    device,
                    state_copy,
                    model,
                    matrix_node_next,
                    min_val, max_val);

                teacher = old_teacher;

                data.alpha_explored += alpha_beta_pair.first * state_copy.prob;
                data.beta_explored += alpha_beta_pair.second * state_copy.prob;
                data.unexplored -= state_copy.prob;
                data.explored_vec.push_back(state_copy.prob.value.get_d());
                data.chance_idx_vec.push_back(chance_idx);

                assert(data.unexplored >= static_cast<typename Types::Probability>(Rational<>{0}));
                if (chance_idx == state_copy.transitions - 1)
                {
                    assert(data.unexplored == static_cast<typename Types::Probability>(Rational<>{0}));
                }

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

                if (data.unexplored > Rational<>{0})
                {
                    // std::cout << '!' << std::endl;
                }

                const typename Types::Real beta_next = data.alpha_explored + alpha * data.unexplored;
                expected_score += row_strategy[i] * beta_next;
            }
            mpq_canonicalize(expected_score.value.get_mpq_t());
            if (expected_score <= beta)
            {
                best_col_idx = col_idx;
                beta = expected_score;
            }
        }
        // mpq_canonicalize(beta.value.get_mpq_t());
        return {best_col_idx, beta};
    }

    // private:
    template <typename T>
    bool fuzzy_equals(T x, T y)
    {
        mpq_ptr a = x.value.get_mpq_t();
        mpq_ptr b = y.value.get_mpq_t();
        mpq_canonicalize(a);
        mpq_canonicalize(b);
        bool answer = mpq_equal(a, b);
        return answer; // TODO
    }

    inline bool try_solve_chance_node(
        typename Types::PRNG &device,
        typename Types::State &state, // TODO const?
        typename Types::Model &model,
        MatrixNode<AlphaBeta> *matrix_node,
        int row_idx,
        int col_idx,
        typename Types::Real alpha,
        typename Types::Real beta)
    {
        MatrixStats &stats = matrix_node->stats;
        CNode<AlphaBeta> *chance_node = matrix_node->access(row_idx, col_idx);
        Data &data = stats.data_matrix.get(row_idx, col_idx);

        if (data.unexplored > typename Types::Rational{0} && !data.solved)
        {

            // typename Types::Action row_action, col_action;
            auto row_action = state.row_actions[row_idx];
            auto col_action = state.col_actions[col_idx];

            auto &chance_actions = data.chance_actions;
            if (chance_actions.size() == 0)
            {
                state.get_chance_actions(chance_actions, row_action, col_action); // TODO arg order
            }

            // go through all chance actions
            for (; data.next_chance_idx < chance_actions.size() && !terminate(device, stats); ++data.next_chance_idx)
            {

                const typename Types::Observation chance_action = chance_actions[data.next_chance_idx];
                typename Types::State state_copy = state;
                state_copy.apply_actions(row_action, col_action, chance_action);

                MNode<AlphaBeta> *matrix_node_next = chance_node->access(state_copy.obs);
                auto old_teacher = teacher;
                teacher = teacher->access(row_idx, col_idx)->access(state_copy.obs);

                auto alpha_beta = double_oracle(device, state_copy, model, matrix_node_next, alpha, beta);
                teacher = old_teacher;
                data.unexplored -= state_copy.prob;

                assert(data.unexplored >= static_cast<typename Types::Probability>(Rational<>{0}));
                if (data.next_chance_idx == state_copy.transitions - 1)
                {
                    assert(data.unexplored == static_cast<typename Types::Probability>(Rational<>{0}));
                }
                data.alpha_explored += alpha_beta.first * state_copy.prob;
                data.beta_explored += alpha_beta.second * state_copy.prob;
            }
        }
        mpq_canonicalize(data.alpha_explored.value.get_mpq_t());
        mpq_canonicalize(data.beta_explored.value.get_mpq_t());

        bool solved_exactly = (data.alpha_explored == data.beta_explored) && (data.unexplored == mpq_class{0, 1});
        data.solved = true;

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