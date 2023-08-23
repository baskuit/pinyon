#pragma once

#include <libsurskit/math.hh>
#include <model/model.hh>

/*
Minimal model for benchmarking purposes (Test speed of state and tree structure)
*/

template <CONCEPT(IsValueModelTypes, Types)>
struct Rand : Types
{
    struct MatrixStats
    {
        int rows, cols;
    };
    struct ChanceStats
    {
    };
    struct Outcome
    {
    };

    class BanditAlgorithm
    {
    public:
        friend std::ostream &operator<<(std::ostream &os, const Rand &session)
        {
            os << "Rand";
            return os;
        }

        void get_empirical_strategies(
            const MatrixStats &stats,
            Types::VectorReal &row_strategy,
            Types::VectorReal &col_strategy) const
        {
        }

        void get_empirical_value(
            const MatrixStats &stats,
            Types::Value &value)
        {
        }

        void get_refined_strategies(
            const MatrixStats &stats,
            Types::VectorReal &row_strategy,
            Types::VectorReal &col_strategy)
        {
        }

        void get_refined_value(
            const MatrixStats &stats,
            Types::Value &value)
        {
        }

        // protected:
        void initialize_stats(
            int iterations,
            Types::State &state,
            Types::Model &model,
            MatrixStats &stats)
        {
        }

        void expand(
            const Types::State &state,
            MatrixStats &stats,
            const Types::ModelOutput &model_output)
        {
            stats.rows = state.row_actions.size();
            stats.cols = state.col_actions.size();
        }

        void select(
            Types::PRNG &device,
            const MatrixStats &stats,
            Outcome &outcome)
        {
            const int row_idx = device.random_int(stats.rows);
            const int col_idx = device.random_int(stats.cols);
            outcome.row_idx = row_idx;
            outcome.col_idx = col_idx;
        }

        void update_matrix_stats(
            MatrixStats &stats,
            const Outcome &outcome)
        {
        }

        void update_chance_stats(
            ChanceStats &stats,
            const Outcome &outcome)
        {
        }

        void select(
            Types::PRNG &device,
            const MatrixStats &stats,
            Outcome &outcome,
            Types::Mutex &mutex)
        {
            const int row_idx = device.random_int(stats.rows);
            const int col_idx = device.random_int(stats.cols);
            outcome.row_idx = row_idx;
            outcome.col_idx = col_idx;
        }

        void update_matrix_stats(
            MatrixStats &stats,
            const Outcome &outcome,
            Types::Mutex &mutex) {}

        void update_chance_stats(
            ChanceStats &stats,
            const Outcome &outcome,
            Types::Mutex &mutex) {}
    };
};
