#pragma once
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <chrono>

#include <types/rational.hh>
#include <types/random.hh>
#include <types/vector.hh>
#include <types/value.hh>
#include <types/matrix.hh>
#include <cstdint>

using ActionIndex = int;

namespace math
{

    template <typename VectorIn, typename VectorOut>
    void power_norm(VectorIn &input, int length, double power, VectorOut &output)
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
    void power_norm(VectorIn &input, double power = 1.0)
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

    template <class Types>
    typename Types::Real exploitability(
        typename Types::MatrixValue &value_matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        const int rows = value_matrix.rows;
        const int cols = value_matrix.cols;

        typename Types::Real row_payoff{typename Types::Rational{0}}, col_payoff{typename Types::Rational{0}};
        typename Types::VectorReal row_response(rows), col_response(cols);
        size_t data_idx = 0;
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                const auto value = value_matrix[data_idx];
                const typename Types::Real u{value.get_row_value() * col_strategy[col_idx]};
                const typename Types::Real v{value.get_col_value() * row_strategy[row_idx]};
                row_payoff += u * row_strategy[row_idx];
                col_payoff += v * col_strategy[col_idx];
                row_response[row_idx] += u;
                col_response[col_idx] += v;
                ++data_idx;
            }
        }

        typename Types::Real row_best_response{*std::max_element(row_response.begin(), row_response.end())};
        typename Types::Real col_best_response{*std::max_element(col_response.begin(), col_response.end())};

        return (row_best_response - row_payoff) + (col_best_response - col_payoff);
    }

    template <class Types>
    typename Types::Real exploitability(
        Matrix<PairDouble> &value_matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        const int rows = value_matrix.rows;
        const int cols = value_matrix.cols;

        typename Types::Real row_payoff{typename Types::Rational{0}}, col_payoff{typename Types::Rational{0}};
        typename Types::VectorReal row_response(rows), col_response(cols);
        size_t data_idx = 0;
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                const auto value = value_matrix[data_idx];
                const typename Types::Real u{value.get_row_value() * static_cast<double>(col_strategy[col_idx])};
                const typename Types::Real v{value.get_col_value() * static_cast<double>(row_strategy[row_idx])};
                row_payoff += u * row_strategy[row_idx];
                col_payoff += v * col_strategy[col_idx];
                row_response[row_idx] += u;
                col_response[col_idx] += v;
                ++data_idx;
            }
        }

        typename Types::Real row_best_response{*std::max_element(row_response.begin(), row_response.end())};
        typename Types::Real col_best_response{*std::max_element(col_response.begin(), col_response.end())};

        return (row_best_response - row_payoff) + (col_best_response - col_payoff);
    }

    double exploitability(
        Matrix<PairDouble> &value_matrix,
        std::vector<double> &row_strategy,
        std::vector<double> &col_strategy)
    {
        const int rows = value_matrix.rows;
        const int cols = value_matrix.cols;

        double row_payoff{0}, col_payoff{0};
        std::vector<double> row_response(rows), col_response(cols);
        size_t data_idx = 0;
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                const auto value = value_matrix[data_idx];
                const double u{value.get_row_value() * static_cast<double>(col_strategy[col_idx])};
                const double v{value.get_col_value() * static_cast<double>(row_strategy[row_idx])};
                row_payoff += u * row_strategy[row_idx];
                col_payoff += v * col_strategy[col_idx];
                row_response[row_idx] += u;
                col_response[col_idx] += v;
                ++data_idx;
            }
        }

        double row_best_response{*std::max_element(row_response.begin(), row_response.end())};
        double col_best_response{*std::max_element(col_response.begin(), col_response.end())};

        return (row_best_response - row_payoff) + (col_best_response - col_payoff);
    }

    template <typename Real, typename Value, template <typename> typename Vector, template <typename> typename Matrix>
    Real EXPL(
        Matrix<Value> &value_matrix,
        Vector<Real> &row_strategy,
        Vector<Real> &col_strategy)
    {
        const int rows = value_matrix.rows;
        const int cols = value_matrix.cols;

        Real row_payoff{Rational<>{0}}, col_payoff{Rational<>{0}};
        Vector<Real> row_response{rows}, col_response{cols};

        size_t data_idx = 0;
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                const Value &value = value_matrix[data_idx];
                const Real u{value.get_row_value() * col_strategy[col_idx]};
                const Real v{value.get_col_value() * row_strategy[row_idx]};
                row_payoff += u * row_strategy[row_idx];
                col_payoff += v * col_strategy[col_idx];
                row_response[row_idx] += u;
                col_response[col_idx] += v;
                ++data_idx;
            }
        }

        Real row_best_response{*std::max_element(row_response.begin(), row_response.end())};
        Real col_best_response{*std::max_element(col_response.begin(), col_response.end())};

        return (row_best_response - row_payoff) + (col_best_response - col_payoff);
    }

} // end 'math'