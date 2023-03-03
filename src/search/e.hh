#pragma once

#include "algorithm.hh"

#include "../libsurskit/math.hh"

template <class Model>
class Exp3p : BanditAlgorithm<Model>
{
    static_assert(std::derived_from<Model, AbstractModel<typename Model::Types::State>> == true);

public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : BanditAlgorithm<Model>::Types
    {
    };
    struct MatrixStats : BanditAlgorithm<Model>::MatrixStats
    {
        int time = 0;
        typename Types::VectorReal row_gains = {0};
        typename Types::VectorReal col_gains = {0};
        typename Types::VectorInt row_visits = {0};
        typename Types::VectorInt col_visits = {0};

        int visits = 0;
        double row_value_total = 0;
        double col_value_total = 0;
    };

    struct ChanceStats : BanditAlgorithm<Model>::ChanceStats
    {
        int visits = 0;
        double row_value_total = 0;
        double col_value_total = 0;
    };

    static void select(
        MatrixNode<Exp3p> *matrix_node,
        typename Types::Outcome &outcome) {}
    static void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Exp3p> *matrix_node){};
    static void init_stats(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Exp3p> *matrix_node) {}
    static void update_matrix_node(typename Types::Outcome &outcome){};
    static void update_chance_node(typename Types::Outcome &outcome){};

private:

    inline void forecast(
        MatrixNode<Exp3p> *matrix_node)
    {
        /*
        Softmaxing of the gains to produce forecasts/strategies for the row and col players.
        The constants eta, gamma, beta are from (arXiv:1204.5721), Theorem 3.3.
        */
        const int time = matrix_node->stats.time;
        const int rows = matrix_node->pair_actions.rows;
        const int cols = matrix_node->pair_actions.cols;
        if (rows == 1)
        {
            this->row_forecast[0] = 1;
        }
        else
        {
            const double eta = .95 * sqrt(log(rows) / (time * rows));
            const double gamma_ = 1.05 * sqrt(rows * log(rows) / time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            this->softmax(this->row_forecast, matrix_node->stats.row_gains, rows, eta);
            for (int row_idx = 0; row_idx < rows; ++row_idx)
            {
                this->row_forecast[row_idx] = 
                    (1 - gamma) * this->row_forecast[row_idx] + 
                    (gamma)*matrix_node->inference_data.row_priors[row_idx];
            }
        }
        if (cols == 1)
        {
            this->col_forecast[0] = 1;
        }
        else
        {
            const double eta = .95 * sqrt(log(cols) / (time * cols));
            const double gamma_ = 1.05 * sqrt(cols * log(cols) / time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            this->softmax(this->col_forecast, matrix_node->stats.col_gains, cols, eta);
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                this->col_forecast[col_idx] =
                    (1 - gamma) * this->col_forecast[col_idx] +
                    (gamma)*matrix_node->inference_data.col_priors[col_idx];
            }
        }
    }

    inline void softmax(
        typename Types::VectorReal &forecast, 
        typename Types::VectorReal &gains, 
        int k, 
        typename Types::Real eta)
    {
        /*
        Softmax helper function with logit scaling and uniform noise.
        */
        double max = 0;
        for (int i = 0; i < k; ++i)
        {
            double x = gains[i];
            if (x > max)
            {
                max = x;
            }
        }
        double sum = 0;
        for (int i = 0; i < k; ++i)
        {
            gains[i] -= max;
            double x = gains[i];
            double y = std::exp(x * eta);
            forecast[i] = y;
            sum += y;
        }
        for (int i = 0; i < k; ++i)
        {
            forecast[i] /= sum;
        }
    };
};