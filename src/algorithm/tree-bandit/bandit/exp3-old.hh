#include <libsurskit/math.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

template <class Model>
class Exp3Old : AbstractAlgorithm<Model>
{

public:
    struct MatrixStats;
    struct ChanceStats;
    struct Outcome;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = Exp3Old::MatrixStats;
        using ChanceStats = Exp3Old::ChanceStats;
        using Outcome = Exp3Old::Outcome;
    };

    struct MatrixStats
    {
        typename Types::VectorReal row_gains;
        typename Types::VectorReal col_gains;
        typename Types::VectorInt row_visits;
        typename Types::VectorInt col_visits;

        int visits = 0;
        typename Types::Value value_total{};
    };

    struct ChanceStats
    {
    };

    struct Outcome
    {
        ActionIndex row_idx, col_idx;
        typename Model::Types::Value value;
        typename Types::Real row_mu, col_mu;
    };

    const typename Types::Real gamma{.01};
    const typename Types::Real one_minus_gamma{gamma * -1 + 1};

    Exp3Old() {}

    Exp3Old(typename Types::Real gamma) : gamma(gamma), one_minus_gamma{gamma * -1 + 1} {}

    friend std::ostream &operator<<(std::ostream &os, const Exp3Old &session)
    {
        os << "Exp3Old; gamma: " << session.gamma;
        return os;
    }

    void get_empirical_strategies(
        MatrixStats &stats,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        row_strategy.fill(stats.row_visits.size());
        col_strategy.fill(stats.col_visits.size());
        math::power_norm(stats.row_visits, row_strategy.size(), 1, row_strategy);
        math::power_norm(stats.col_visits, col_strategy.size(), 1, col_strategy);
    }

    void get_empirical_value(
        MatrixStats &stats,
        typename Types::Value &value)
    {
        const typename Types::Real den = 1 / (stats.total_visits + (stats.total_visits == 0));
        value = stats.value_total * den;
    }

    void get_refined_strategies(
        MatrixStats &stats,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        row_strategy.fill(stats.row_visits.size());
        col_strategy.fill(stats.col_visits.size());
        denoise(row_strategy, col_strategy); // TODO
        math::power_norm(stats.row_visits, row_strategy.size(), 1, row_strategy);
        math::power_norm(stats.col_visits, col_strategy.size(), 1, col_strategy);
    }

    void get_refined_value(
        MatrixStats &stats,
        typename Types::Value &value)
    {
        get_empirical_value(stats, value);
    }

protected:
    void initialize_stats(
        int iterations,
        const typename Types::State &state,
        typename Types::Model &model,
        MatrixStats &stats)
    {
    }

    void expand(
        typename Types::State &state,
        MatrixStats &stats,
        typename Types::ModelOutput &inference)
    {
        stats.row_visits.fill(state.row_actions.size(), 0);
        stats.col_visits.fill(state.col_actions.size(), 0);
        stats.row_gains.fill(state.row_actions.size(), 0);
        stats.col_gains.fill(state.col_actions.size(), 0);
    }

    void select(
        typename Types::PRNG &device,
        MatrixStats &stats,
        Outcome &outcome)
    {
        const int rows = stats.row_visits.size();
        const int cols = stats.col_visits.size();
        typename Types::VectorReal row_forecast(rows);
        typename Types::VectorReal col_forecast(cols);
        if (rows == 1)
        {
            row_forecast[0] = 1;
        }
        else
        {
            const typename Types::Real eta {gamma / static_cast<typename Types::Real>(rows)};
            softmax(row_forecast, stats.row_gains, rows, eta);
            for (int row_idx = 0; row_idx < rows; ++row_idx)
            {
                row_forecast[row_idx] = (gamma * -1 + 1) * row_forecast[row_idx] + eta;
            }
        }
        if (cols == 1)
        {
            col_forecast[0] = 1;
        }
        else
        {
            const typename Types::Real eta {gamma / static_cast<typename Types::Real>(cols)};
            softmax(col_forecast, stats.col_gains, cols, eta);
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                col_forecast[col_idx] = (gamma * -1 + 1) * col_forecast[col_idx] + eta;
            }
        }
        const int row_idx = device.sample_pdf(row_forecast, rows);
        const int col_idx = device.sample_pdf(col_forecast, cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
        outcome.row_mu = static_cast<typename Types::Real>(row_forecast[row_idx]);
        outcome.col_mu = static_cast<typename Types::Real>(col_forecast[col_idx]);
    }

    void update_matrix_stats(
        MatrixStats &stats,
        Outcome &outcome)
    {
        stats.value_total += outcome.value;
        stats.visits += 1;
        stats.row_visits[outcome.row_idx] += 1;
        stats.col_visits[outcome.col_idx] += 1;
        stats.row_gains[outcome.row_idx] += outcome.value.get_row_value() / outcome.row_mu;
        stats.col_gains[outcome.col_idx] += outcome.value.get_col_value() / outcome.col_mu;
    }

    void update_chance_stats(
        ChanceStats &stats,
        Outcome &outcome)
    {
    }

    // multithreaded

    void select(
        typename Types::PRNG &device,
        MatrixStats &stats,
        Outcome &outcome,
        std::mutex &mtx)
    {
        mtx.lock();
        typename Types::VectorReal row_forecast{stats.row_gains};
        typename Types::VectorReal col_forecast{stats.col_gains};
        mtx.unlock();
        const int rows = row_forecast.size();
        const int cols = col_forecast.size();
        const auto &one_minus_gamma = this->one_minus_gamma;

        if (rows == 1)
        {
            row_forecast[0] = 1;
        }
        else
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(rows)};
            softmax(row_forecast, row_forecast, rows, eta);
            std::transform(row_forecast.begin(), row_forecast.begin() + rows, row_forecast.begin(),
                           [eta, one_minus_gamma](typename Types::Real value)
                           { return one_minus_gamma * value + eta; });
        }
        if (cols == 1)
        {
            col_forecast[0] = 1;
        }
        else
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(cols)};
            softmax(col_forecast, col_forecast, cols, eta);
            std::transform(col_forecast.begin(), col_forecast.begin() + cols, col_forecast.begin(),
                           [eta, one_minus_gamma](typename Types::Real value)
                           { return one_minus_gamma * value + eta; });
        }
        const int row_idx = device.sample_pdf(row_forecast, rows);
        const int col_idx = device.sample_pdf(col_forecast, cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
        outcome.row_mu = static_cast<typename Types::Real>(row_forecast[row_idx]);
        outcome.col_mu = static_cast<typename Types::Real>(col_forecast[col_idx]);
    }

    void update_matrix_stats(
        MatrixStats &stats,
        Outcome &outcome,
        std::mutex &mtx)
    {
        mtx.lock();
        stats.value_total += outcome.value;
        stats.visits += 1;
        stats.row_visits[outcome.row_idx] += 1;
        stats.col_visits[outcome.col_idx] += 1;
        if ((stats.row_gains[outcome.row_idx] += outcome.value.get_row_value() / outcome.row_mu) >= 0)
        {
            const auto max = stats.row_gains[outcome.row_idx];
            for (auto &v : stats.row_gains)
            {
                v -= max;
            }
        }
        if ((stats.col_gains[outcome.col_idx] += outcome.value.get_col_value() / outcome.col_mu) >= 0)
        {
            const auto max = stats.col_gains[outcome.col_idx];
            for (auto &v : stats.col_gains)
            {
                v -= max;
            }
        }
        mtx.unlock();
    }

    void update_chance_stats(
        ChanceStats &stats,
        Outcome &outcome,
        std::mutex &mtx)
    {
    }

    // off-policy

    void update_matrix_stats(
        MatrixStats &stats,
        Outcome &outcome,
        typename Types::Real learning_rate)
    {
        stats.value_total += outcome.value * learning_rate;
        stats.visits += 1;
        stats.row_visits[outcome.row_idx] += 1;
        stats.col_visits[outcome.col_idx] += 1;
        stats.row_gains[outcome.row_idx] += outcome.value.get_row_value() / outcome.row_mu * learning_rate;
        stats.col_gains[outcome.col_idx] += outcome.value.get_col_value() / outcome.col_mu * learning_rate;
    }

    void update_chance_stats(
        ChanceStats &stats,
        Outcome &outcome,
        typename Types::Real learning_rate)
    {
    }

    void get_policy(
        MatrixStats &stats,
        typename Types::VectorReal &row_policy,
        typename Types::VectorReal &col_policy)
    {
        const int rows = stats.row_gains.size();
        const int cols = stats.col_gains.size();
        const auto &one_minus_gamma = this->one_minus_gamma;
        if (rows == 1)
        {
            row_policy[0] = 1;
        }
        else
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(rows)};
            softmax(row_policy, stats.row_gains, rows, eta);
            std::transform(row_policy.begin(), row_policy.begin() + rows, row_policy.begin(),
                           [eta, one_minus_gamma](typename Types::Real value)
                           { return one_minus_gamma * value + eta; });
        }
        if (cols == 1)
        {
            col_policy[0] = 1;
        }
        else
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(cols)};
            softmax(col_policy, stats.col_gains, cols, eta);
            std::transform(col_policy.begin(), col_policy.begin() + cols, col_policy.begin(),
                           [eta, one_minus_gamma](typename Types::Real value)
                           { return one_minus_gamma * value + eta; });
        }
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
        typename Types::Real max{0};
        for (int i = 0; i < k; ++i)
        {
            typename Types::Real x{gains[i]};
            if (x > max)
            {
                max = x;
            }
        }
        typename Types::Real sum{0};
        for (int i = 0; i < k; ++i)
        {
            gains[i] -= max;
            typename Types::Real x{gains[i]};
            typename Types::Real y{std::exp(static_cast<double>(x * eta))};
            forecast[i] = y;
            sum += y;
        }
        for (int i = 0; i < k; ++i)
        {
            forecast[i] /= sum;
        }
    };

    inline void denoise(
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        const int rows = row_strategy.size();
        const int cols = col_strategy.size();
        const auto &one_minus_gamma = this->one_minus_gamma;
        if (rows > 1)
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(rows)};
            std::transform(row_strategy.begin(), row_strategy.begin() + rows, row_strategy.begin(),
                           [eta, one_minus_gamma](typename Types::Real value)
                           { return (value - eta) / one_minus_gamma; });
        }
        if (cols > 1)
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(cols)};
            std::transform(col_strategy.begin(), col_strategy.begin() + cols, col_strategy.begin(),
                           [eta, one_minus_gamma](typename Types::Real value)
                           { return (value - eta) / one_minus_gamma; });
        }
    } // TODO can produce negative values but this shouldnt cause problems I think.
};