#pragma once

#include <libsurskit/math.hh>

/*
Minimal model for benchmarking purposes (Test speed of state and tree structure)
*/

template <class Model>
class Rand : public AbstractAlgorithm<Model>
{

public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = Rand::MatrixStats;
        using ChanceStats = Rand::ChanceStats;
    };
    struct MatrixStats
    {
        int rows, cols;
    };

    struct ChanceStats
    {
    };

    Rand() {}

    friend std::ostream &operator<<(std::ostream &os, const Rand &session)
    {
        os << "Rand";
        return os;
    }

    void get_empirical_strategies(
        MatrixStats &stats,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
    }

    void get_empirical_values(
        MatrixStats &stats,
        typename Types::Real &row_value,
        typename Types::Real &col_value)
    {
    }

    void get_refined_strategies(
        MatrixStats &stats,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
    }

    void get_refined_values(
        MatrixStats &stats,
        typename Types::Real &row_value,
        typename Types::Real &col_value)
    {
    }
protected:

    void initialize_stats(
        int iterations,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixStats &stats)
    {
    }

    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixStats &stats)
    {
        stats.rows = state.row_actions.size();
        stats.cols = state.col_actions.size();
    }

    void select(
        typename Types::PRNG &device,
        MatrixStats &stats,
        typename Types::Outcome &outcome)
    {
        const int row_idx = device.random_int(stats.rows);
        const int col_idx = device.random_int(stats.cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
    }

    void update_matrix_stats(
        MatrixStats &stats,
        typename Types::Outcome &outcome)
    {
    }

    void update_chance_stats(
        ChanceStats &stats,
        typename Types::Outcome &outcome)
    {
    }
};