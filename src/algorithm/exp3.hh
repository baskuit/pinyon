#pragma once

#include "algorithm.hh"

#include "../libsurskit/math.hh"

/*
Exp3
*/

template <class Model, template <class _Model, class _BanditAlgorithm> class _TreeBandit>
class Exp3 : public _TreeBandit<Model, Exp3<Model, _TreeBandit>>
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : _TreeBandit<Model, Exp3<Model, _TreeBandit>>::Types
    {
        using MatrixStats = Exp3::MatrixStats;
        using ChanceStats = Exp3::ChanceStats;
    };
    struct MatrixStats : _TreeBandit<Model, Exp3<Model, _TreeBandit>>::MatrixStats
    {
        typename Types::VectorReal row_gains;
        typename Types::VectorReal col_gains;
        typename Types::VectorInt row_visits;
        typename Types::VectorInt col_visits;

        int visits = 0;
        typename Types::Real row_value_total = 0;
        typename Types::Real col_value_total = 0;
    };

    struct ChanceStats : _TreeBandit<Model, Exp3<Model, _TreeBandit>>::ChanceStats
    {
        int visits = 0;
        typename Types::Real row_value_total = 0;
        typename Types::Real col_value_total = 0;
    };

    typename Types::Real gamma = .01;

    Exp3() {}

    Exp3(typename Types::Real gamma) : gamma(gamma) {}

    friend std::ostream &operator<<(std::ostream &os, const Exp3 &session)
    {
        os << "Exp3p; gamma: " << session.gamma;
        return os;
    }

    void get_strategies(
        MatrixNode<Exp3> *matrix_node,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        math::power_norm<typename Types::VectorInt, typename Types::VectorReal>(
            matrix_node->stats.row_visits,
            matrix_node->actions.rows,
            1.0,
            row_strategy);
        math::power_norm<typename Types::VectorInt, typename Types::VectorReal>(
            matrix_node->stats.col_visits,
            matrix_node->actions.cols,
            1.0,
            col_strategy);
    }

    void initialize_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Exp3> *matrix_node)
    {
    }

    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Exp3> *matrix_node)
    {
        state.get_actions();
        matrix_node->actions = state.actions;
        matrix_node->is_expanded = true;
        matrix_node->is_terminal = state.is_terminal;

        matrix_node->stats.row_visits.fill(state.actions.rows, 0);
        matrix_node->stats.col_visits.fill(state.actions.cols, 0);
        matrix_node->stats.row_gains.fill(state.actions.rows, 0);
        matrix_node->stats.col_gains.fill(state.actions.cols, 0);

        if (matrix_node->is_terminal)
        {
            matrix_node->inference.row_value = state.row_payoff;
            matrix_node->inference.col_value = state.col_payoff;
        }
        else
        {
            model.get_inference(state, matrix_node->inference);
        }
    }

    void select(
        prng &device,
        MatrixNode<Exp3> *matrix_node,
        typename Types::Outcome &outcome)
    {
        /*
        Softmaxing of the gains to produce forecasts/strategies for the row and col players.
        The constants eta, gamma, beta are from (arXiv:1204.5721), Theorem 3.3.
        */
        const int rows = matrix_node->actions.rows;
        const int cols = matrix_node->actions.cols;
        typename Types::VectorReal row_forecast(rows);
        typename Types::VectorReal col_forecast(cols);
        if (rows == 1)
        {
            row_forecast[0] = 1;
        }
        else
        {
            const typename Types::Real eta = gamma / rows;
            softmax(row_forecast, matrix_node->stats.row_gains, rows, eta);
            for (int row_idx = 0; row_idx < rows; ++row_idx)
            {
                row_forecast[row_idx] = (1 - gamma) * row_forecast[row_idx] + eta;
            }
        }
        if (cols == 1)
        {
            col_forecast[0] = 1;
        }
        else
        {
            const typename Types::Real eta = gamma / cols;
            softmax(col_forecast, matrix_node->stats.col_gains, cols, eta);
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                col_forecast[col_idx] = (1 - gamma) * col_forecast[col_idx] + eta;
            }
        }
        const int row_idx = device.sample_pdf(row_forecast, rows);
        const int col_idx = device.sample_pdf(col_forecast, cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
        outcome.row_mu = row_forecast[row_idx];
        outcome.col_mu = col_forecast[col_idx];
    }

    void update_matrix_node(
        MatrixNode<Exp3> *matrix_node,
        typename Types::Outcome &outcome)
    {
        matrix_node->stats.row_value_total += outcome.row_value;
        matrix_node->stats.col_value_total += outcome.col_value;
        matrix_node->stats.visits += 1;
        matrix_node->stats.row_visits[outcome.row_idx] += 1;
        matrix_node->stats.col_visits[outcome.col_idx] += 1;
        matrix_node->stats.row_gains[outcome.row_idx] += outcome.row_value / outcome.row_mu;
        matrix_node->stats.col_gains[outcome.col_idx] += outcome.col_value / outcome.col_mu;
    }

    void update_chance_node(
        ChanceNode<Exp3> *chance_node,
        typename Types::Outcome &outcome)
    {
        chance_node->stats.row_value_total += outcome.row_value;
        chance_node->stats.col_value_total += outcome.col_value;
        chance_node->stats.visits += 1;
    }

private:
    inline void softmax(
        typename Types::VectorReal &forecast,
        typename Types::VectorReal &gains,
        int k,
        typename Types::Real eta)
    {
        /*
        Softmax helper function with logit scaling.
        */
        typename Types::Real max = 0;
        for (int i = 0; i < k; ++i)
        {
            typename Types::Real x = gains[i];
            if (x > max)
            {
                max = x;
            }
        }
        typename Types::Real sum = 0;
        for (int i = 0; i < k; ++i)
        {
            gains[i] -= max;
            typename Types::Real x = gains[i];
            typename Types::Real y = std::exp(x * eta);
            forecast[i] = y;
            sum += y;
        }
        for (int i = 0; i < k; ++i)
        {
            forecast[i] /= sum;
        }
    };
};