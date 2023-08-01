#pragma once

#include <libsurskit/math.hh>

/*
Minimal model for benchmarking purposes (Test speed of state and tree structure)
*/

template <IsValueModelTypes Types>
class Rand
{
public:
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
    struct T : Types {
        using BanditAlgorithm = Rand;
        using Rand::MatrixStats;
        using Rand::ChanceStats;
        using Rand::Outcome;
    };

    friend std::ostream &operator<<(std::ostream &os, const Rand &session)
    {
        os << "Rand";
        return os;
    }

    void get_empirical_strategies(
        MatrixStats &stats,
        Types::VectorReal &row_strategy,
        Types::VectorReal &col_strategy) const
    {
    }

    void get_empirical_value(
        MatrixStats &stats,
        Types::Value &value)
    {
    }

    void get_refined_strategies(
        MatrixStats &stats,
        Types::VectorReal &row_strategy,
        Types::VectorReal &col_strategy)
    {
    }

    void get_refined_value(
        MatrixStats &stats,
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
        Types::State &state,
        MatrixStats &stats,
        Types::ModelOutput &inference)
    {
        stats.rows = state.row_actions.size();
        stats.cols = state.col_actions.size();
    }

    void select(
        Types::PRNG &device,
        MatrixStats &stats,
        Types::Outcome &outcome)
    {
        const int row_idx = device.random_int(stats.rows);
        const int col_idx = device.random_int(stats.cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
    }

    void update_matrix_stats(
        MatrixStats &stats,
        Types::Outcome &outcome)
    {
    }

    void update_chance_stats(
        ChanceStats &stats,
        Types::Outcome &outcome)
    {
    }

    void select(
        Types::PRNG &device,
        MatrixStats &stats,
        Types::Outcome &outcome,
        Types::Mutex &mtx)
    {
        const int row_idx = device.random_int(stats.rows);
        const int col_idx = device.random_int(stats.cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
    }

    void update_matrix_stats(
        MatrixStats &stats,
        Types::Outcome &outcome,
        Types::Mutex &mtx) {}

    void update_chance_stats(
        ChanceStats &stats,
        Types::Outcome &outcome,
        Types::Mutex &mtx) {}

    void update_matrix_stats(
        MatrixStats &stats,
        Types::Outcome &outcome,
        Types::Real learning_rate)
    {
    }

    void update_chance_stats(
        ChanceStats &stats,
        Types::Outcome &outcome,
        Types::Real learning_rate)
    {
    }

    void get_policy(
        MatrixStats &stats,
        Types::VectorReal &row_policy,
        Types::VectorReal &col_policy)
    {
        const typename Types::Real row_uniform{Rational{1, stats.rows}};
        row_policy.resize(stats.rows, row_uniform);
        const typename Types::Real col_uniform{Rational{1, stats.cols}};
        col_policy.resize(stats.cols, col_uniform);
    }
};