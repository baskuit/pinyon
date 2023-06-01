#pragma once
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <chrono>

#include <types/random.hh>
#include <types/random.hh>
#include <libsurskit/vector.hh>

using ActionIndex = int;

namespace math
{

    template <typename VectorIn, typename VectorOut>
    void power_norm(VectorIn &input, int length, double power, VectorOut &output)

    // TODO modernize
    {
        double sum = 0;
        for (int i = 0; i < length; ++i)
        {
            double x = std::pow(input[i], power);
            output[i] = x;
            sum += x;
        }
        for (int i = 0; i < length; ++i)
        {
            output[i] = output[i] / sum;
        }
    }

    template <typename VectorIn>
    void power_norm(VectorIn &input, double power=1.0)

    // TODO modernize
    {
        double sum = 0;
        const size_t length = input.size();
        for (int i = 0; i < length; ++i)
        {
            double x = std::pow(input[i], power);
            input[i] = x;
            sum += x;
        }
        for (int i = 0; i < length; ++i)
        {
            input[i] = input[i] / sum;
        }
    }


    template <typename Vector>
    void print(Vector &input)
    {
        for (int i = 0; i < input.size(); ++i)
        {
            std::cout << input[i] << ", ";
        }
        std::cout << '\n';
    }

    template <typename Real>
    Real sigmoid (Real x) {
        return 1 / (1 + exp(x));
    }
}

    template <class Types>
    typename Types::Real exploitability(
        typename Types::MatrixValue &value_matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        const int rows = value_matrix.rows;
        const int cols = value_matrix.cols;

        typename Types::Real row_payoff{Rational(0)}, col_payoff{Rational(0)}; // TODO rationals?
        typename Types::VectorReal row_response(rows), col_response(cols);
        size_t data_idx = 0;
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                const auto value = value_matrix[data_idx];
                const typename Types::Real u{value.row_value * col_strategy[col_idx]};
                const typename Types::Real v{value.col_value * row_strategy[row_idx]};
                row_payoff += u * row_strategy[row_idx];
                col_payoff += v * col_strategy[col_idx];
                row_response[row_idx] += u;
                col_response[col_idx] += v;
                ++data_idx;
            }
        }

        typename Types::Real row_best_response {*std::max_element(row_response.begin(), row_response.end())};
        typename Types::Real col_best_response {*std::max_element(col_response.begin(), col_response.end())};

        return (row_best_response - row_payoff) + (col_best_response - col_payoff);
    }
