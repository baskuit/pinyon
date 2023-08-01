#include <libsurskit/math.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

template <IsValueModelTypes Types>
class Exp3
{
public:
    struct MatrixStats
    {
        typename Types::VectorReal row_gains;
        typename Types::VectorReal col_gains;
        typename Types::VectorInt row_visits;
        typename Types::VectorInt col_visits;

        int visits = 0;
        PairReal<typename Types::Real> value_total{0, 0};
    };
    struct ChanceStats
    {
    };
    struct Outcome
    {
        ActionIndex row_idx, col_idx;
        typename Types::Value value;
        typename Types::Real row_mu, col_mu;
    };

    const typename Types::Real gamma{.01};
    const typename Types::Real one_minus_gamma{gamma * -1 + 1};

    Exp3() {}

    constexpr Exp3(Types::Real gamma) : gamma(gamma), one_minus_gamma{gamma * -1 + 1} {}

    friend std::ostream &operator<<(std::ostream &os, const Exp3 &session)
    {
        os << "Exp3; gamma: " << session.gamma;
        return os;
    }

    void get_empirical_strategies(
        MatrixStats &stats,
        Types::VectorReal &row_strategy,
        Types::VectorReal &col_strategy) const
    {
        row_strategy.resize(stats.row_visits.size());
        col_strategy.resize(stats.col_visits.size());
        math::power_norm(stats.row_visits, row_strategy.size(), 1, row_strategy);
        math::power_norm(stats.col_visits, col_strategy.size(), 1, col_strategy);
    }

    void get_empirical_value(
        MatrixStats &stats,
        Types::Value &value) const
    {
        const typename Types::Real den = 1 / (stats.total_visits + (stats.total_visits == 0));
        value = stats.value_total * den;
    }

    void get_refined_strategies(
        MatrixStats &stats,
        Types::VectorReal &row_strategy,
        Types::VectorReal &col_strategy) const
    {
        row_strategy.resize(stats.row_visits.size());
        col_strategy.resize(stats.col_visits.size());
        denoise(row_strategy, col_strategy);
        math::power_norm(stats.row_visits, row_strategy.size(), 1, row_strategy);
        math::power_norm(stats.col_visits, col_strategy.size(), 1, col_strategy);
    }

    void get_refined_value(
        MatrixStats &stats,
        Types::Value &value) const
    {
        get_empirical_value(stats, value);
    }

// protected:
    void initialize_stats(
        int iterations,
        const Types::State &state,
        Types::Model &model,
        MatrixStats &stats) const
    {
    }

    void expand(
        Types::State &state,
        MatrixStats &stats,
        Types::ModelOutput &inference)
    {
        stats.row_visits.resize(state.row_actions.size(), 0);
        stats.col_visits.resize(state.col_actions.size(), 0);
        stats.row_gains.resize(state.row_actions.size(), 0);
        stats.col_gains.resize(state.col_actions.size(), 0);
    }

    void select(
        Types::PRNG &device,
        MatrixStats &stats,
        Outcome &outcome) const
    {
        const int rows = stats.row_gains.size();
        const int cols = stats.col_gains.size();
        const auto &one_minus_gamma = this->one_minus_gamma;
        typename Types::VectorReal row_forecast(rows);
        typename Types::VectorReal col_forecast(cols);
        if (rows == 1)
        {
            row_forecast[0] = Rational<>{1};
        }
        else
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(rows)};
            softmax(row_forecast, stats.row_gains, rows, eta);
            std::transform(row_forecast.begin(), row_forecast.begin() + rows, row_forecast.begin(),
                           [eta, one_minus_gamma](typename Types::Real value)
                           { return one_minus_gamma * value + eta; });
        }
        if (cols == 1)
        {
            col_forecast[0] = Rational<>{1};
        }
        else
        {
            const typename Types::Real eta{gamma / static_cast<typename Types::Real>(cols)};
            softmax(col_forecast, stats.col_gains, cols, eta);
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
        Outcome &outcome) const
    {
        stats.value_total += PairReal<typename Types::Real>{outcome.value.get_row_value(), outcome.value.get_col_value()};
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
    }

    void update_chance_stats(
        ChanceStats &stats,
        Outcome &outcome) const
    {
    }

    // multithreaded

    void select(
        Types::PRNG &device,
        MatrixStats &stats,
        Outcome &outcome,
        Types::Mutex &mtx) const
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
            row_forecast[0] = Rational<>{1};
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
            col_forecast[0] = Rational<>{1};
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
        Types::Mutex &mtx) const
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
        Types::Mutex &mtx)
    {
    }

    // off-policy

    void update_matrix_stats(
        MatrixStats &stats,
        Outcome &outcome,
        Types::Real learning_rate) const
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
        Types::Real learning_rate) const
    {
    }

    void get_policy(
        MatrixStats &stats,
        Types::VectorReal &row_policy,
        Types::VectorReal &col_policy) const
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
        Types::VectorReal &forecast,
        Types::VectorReal &gains,
        const size_t k,
        Types::Real eta) const
    {
        typename Types::Real sum = 0;
        for (size_t i = 0; i < k; ++i)
        {
            const typename Types::Real y{std::exp(static_cast<double>(gains[i] * eta))};
            forecast[i] = y;
            sum += y;
        }
        for (size_t i = 0; i < k; ++i)
        {
            forecast[i] /= sum;
        }
    };

    inline void denoise(
        Types::VectorReal &row_strategy,
        Types::VectorReal &col_strategy)
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