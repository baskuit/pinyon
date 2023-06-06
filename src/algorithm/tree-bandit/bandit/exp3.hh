#pragma once

#include <libsurskit/math.hh>
#include <tree/tree.hh>
#include <tree/tree-obs.hh>

/*
Exp3
*/

template <
    class Model,
    template <class _Model, class _BanditAlgorithm, class Outcome, template <class A> class MNode, template <class A> class CNode> class _TreeBandit = TreeBandit,
    template <class Algo> class _MatrixNode = MatrixNodeL,
    template <class Algo> class _ChanceNode = ChanceNodeL
>
class Exp3 : public _TreeBandit<Model, Exp3<Model, _TreeBandit>, ChoicesOutcome<Model>, _MatrixNode, _ChanceNode>
{

public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : _TreeBandit<Model, Exp3<Model, _TreeBandit>, ChoicesOutcome<Model>, _MatrixNode, _ChanceNode>::Types
    {
        using MatrixStats = Exp3::MatrixStats;
        using ChanceStats = Exp3::ChanceStats;
        using MatrixNode = _MatrixNode<Exp3>;
        using ChanceNode = _ChanceNode<Exp3>;
    };

    struct MatrixStats : _TreeBandit<Model, Exp3<Model, _TreeBandit>, ChoicesOutcome<Model>, _MatrixNode, _ChanceNode>::MatrixStats
    {
        typename Types::VectorReal row_gains;
        typename Types::VectorReal col_gains;
        typename Types::VectorInt row_visits;
        typename Types::VectorInt col_visits;

        int visits = 0;
        typename Types::Value value_total;

    };

    struct ChanceStats : _TreeBandit<Model, Exp3<Model, _TreeBandit>, ChoicesOutcome<Model>, _MatrixNode, _ChanceNode>::ChanceStats
    {
    };

    typename Types::Real gamma{.01};

    Exp3() {}

    Exp3(typename Types::Real gamma) : gamma(gamma) {}

    friend std::ostream &operator<<(std::ostream &os, const Exp3 &session)
    {
        os << "Exp3; gamma: " << session.gamma;
        return os;
    }

    void get_empirical_strategies(
        _MatrixNode<Exp3> *matrix_node,
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
        _MatrixNode<Exp3> *matrix_node,
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
        _MatrixNode<Exp3> *matrix_node)
    {
    }

    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        _MatrixNode<Exp3> *matrix_node)
    {
        matrix_node->stats.row_visits.fill(state.row_actions.size(), 0);
        matrix_node->stats.col_visits.fill(state.col_actions.size(), 0);
        matrix_node->stats.row_gains.fill(state.row_actions.size(), 0);
        matrix_node->stats.col_gains.fill(state.col_actions.size(), 0);
    }

    void select(
        typename Types::PRNG &device,
        _MatrixNode<Exp3> *matrix_node,
        typename Types::Outcome &outcome)
    {
        /*
        Softmaxing of the gains to produce forecasts/strategies for the row and col players.
        The constants eta, gamma, beta are from (arXiv:1204.5721), Theorem 3.3.
        */
        const int rows = matrix_node->row_actions.size();
        const int cols = matrix_node->col_actions.size();
        typename Types::VectorReal row_forecast(rows);
        typename Types::VectorReal col_forecast(cols);
        if (rows == 1)
        {
            row_forecast[0] = 1;
        }
        else
        {
            const typename Types::Real eta {gamma / static_cast<typename Types::Real>(rows)};
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
            const typename Types::Real eta {gamma / static_cast<typename Types::Real>(cols)};
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
        outcome.row_mu = static_cast<typename Types::Real>(row_forecast[row_idx]);
        outcome.col_mu = static_cast<typename Types::Real>(col_forecast[col_idx]);
    }

    void update_matrix_node(
        _MatrixNode<Exp3> *matrix_node,
        typename Types::Outcome &outcome)
    {
        matrix_node->stats.value_total += outcome.value;
        matrix_node->stats.visits += 1;
        matrix_node->stats.row_visits[outcome.row_idx] += 1;
        matrix_node->stats.col_visits[outcome.col_idx] += 1;
        matrix_node->stats.row_gains[outcome.row_idx] += outcome.value.get_row_value() / outcome.row_mu;
        matrix_node->stats.col_gains[outcome.col_idx] += outcome.value.get_col_value() / outcome.col_mu;
    }

    void update_chance_node(
        _ChanceNode<Exp3> *chance_node,
        typename Types::Outcome &outcome)
    {
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
            typename Types::Real y{std::exp(x * eta)};
            forecast[i] = y;
            sum += y;
        }
        for (int i = 0; i < k; ++i)
        {
            forecast[i] /= sum;
        }
    };

    inline void denoise (
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy
    ) {
        // TODO
    }
};