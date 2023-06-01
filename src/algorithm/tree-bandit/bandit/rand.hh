#pragma once

// #include "bandit.hh>

#include <libsurskit/math.hh>

#include <type_traits>

/*
Rand
*/

template <class Model, template <class _Model, class _BanditAlgorithm, class Outcome> 
    class _TreeBandit = TreeBandit, typename = void>
class Rand : public _TreeBandit<Model, Rand<Model, _TreeBandit>, ChoicesOutcome<Model>>
{

public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : _TreeBandit<Model, Rand<Model, _TreeBandit>,  ChoicesOutcome<Model>>::Types
    {
        using MatrixStats = Rand::MatrixStats;
        using ChanceStats = Rand::ChanceStats;
    };
    struct MatrixStats : _TreeBandit<Model, Rand<Model, _TreeBandit>, ChoicesOutcome<Model>>::MatrixStats
    {
    };

    struct ChanceStats : _TreeBandit<Model, Rand<Model, _TreeBandit>, ChoicesOutcome<Model>>::ChanceStats
    {
        // int visits = 0;
        // typename Types::Real row_value_total{0};
        // typename Types::Real col_value_total{0};
        // Without this information, we cannot determine value/visit matrix
        // However we don't currently use that info anyway TODO
    };

    typename Types::Real gamma{.01};

    Rand() {}

    Rand(typename Types::Real gamma) : gamma(gamma) {}

    friend std::ostream &operator<<(std::ostream &os, const Rand &session)
    {
        os << "Rand; gamma: " << session.gamma;
        return os;
    }

    void get_empirical_strategies(
        MatrixNode<Rand> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        // row_strategy.fill(matrix_node->stats.row_visits.size());
        // col_strategy.fill(matrix_node->stats.col_visits.size());
        // // denoise? TODO
        // denoise(row_strategy, col_strategy); // uses this->gamma;
        // math::power_norm(matrix_node->stats.row_visits, row_strategy.size(), 1, row_strategy);
        // math::power_norm(matrix_node->stats.col_visits, col_strategy.size(), 1, col_strategy);
    }

    void get_empirical_values(
        MatrixNode<Rand> *matrix_node,
        typename Types::Real &row_value,
        typename Types::Real &col_value)
    {
        // auto stats = matrix_node->stats;
        // const typename Types::Real den = 1 / (stats.total_visits + (stats.total_visits == 0));
        // row_value = stats.row_value_total * den;
        // col_value = stats.col_value_total * den;
    }

    void initialize_stats(
        int iterations,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Rand> *matrix_node)
    {
    }

    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Rand> *matrix_node)
    {
        // const size_t rows_cols = state.row_actions.size() + state.col_actions.size();
        // matrix_node->stats.row_col_visits.fill(rows_cols, 0);
        // matrix_node->stats.row_col_gains.fill(rows_cols, 0);
    }

    void select(
        typename Types::PRNG &device,
        MatrixNode<Rand> *matrix_node,
        typename Types::Outcome &outcome)
    {
        const int rows = matrix_node->row_actions.size();
        const int cols = matrix_node->col_actions.size();
        const int row_idx = device.random_int(rows);
        const int col_idx = device.random_int(cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
        outcome.row_mu = static_cast<typename Types::Real>(1 / (double) rows);
        outcome.col_mu = static_cast<typename Types::Real>(1 / (double) cols);
    }

    void update_matrix_node(
        MatrixNode<Rand> *matrix_node,
        typename Types::Outcome &outcome)
    {
        // const size_t rows = matrix_node->row_actions.size();
        // matrix_node->stats.value_total += outcome.value;
        // matrix_node->stats.visits += 1;
        // matrix_node->stats.row_col_visits[outcome.row_idx] += 1;
        // matrix_node->stats.row_col_visits[outcome.col_idx + rows] += 1;
        // matrix_node->stats.row_col_gains[outcome.row_idx] += outcome.value.get_row_value() / outcome.row_mu;
        // matrix_node->stats.row_col_gains[outcome.col_idx + rows] += outcome.value.get_col_value() / outcome.col_mu;
    }

    void update_chance_node(
        ChanceNode<Rand> *chance_node,
        typename Types::Outcome &outcome)
    {
        // chance_node->stats.row_value_total += outcome.row_value;
        // chance_node->stats.col_value_total += outcome.col_value;
        // chance_node->stats.visits += 1;
    }
};