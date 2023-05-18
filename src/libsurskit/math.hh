#pragma once
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "random.hh"
#include "rational.hh"
#include "vector.hh"

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

namespace Linear
{
    template <typename T>
    class Matrix
    {
    public:
        std::vector<T> data;
        int rows, cols;

        Matrix(){};
        Matrix(int rows, int cols) : data(std::vector<T>(rows * cols)), rows(rows), cols(cols)
        {
        }

        void fill(int rows, int cols)
        {
            this->rows = rows;
            this->cols = cols;
            data.resize(rows * cols);
        }

        void fill(int rows, int cols, T value)
        {
            this->rows = rows;
            this->cols = cols;
            const int n = rows * cols;
            data.resize(n);
            std::fill(data.begin(), data.begin() + n, value);
        }

        T &get(int i, int j)
        {
            return data[i * cols + j];
        }

        Matrix operator*(T t)
        {
            const Matrix &M = *this;
            Matrix output(M.rows, M.cols);
            for (int i = 0; i < rows * cols; ++i)
            {
                output.data[i] = M.data[i] * t;
            }
            return output;
        }
        Matrix operator+(T t)
        {
            const Matrix &M = *this;
            Matrix output(M.rows, M.cols);
            for (int i = 0; i < rows * cols; ++i)
            {
                output.data[i] = M.data[i] + t;
            }
            return output;
        }

        Matrix operator+(const Matrix &t)
        {
            assert(t.rows == rows && t.cols == cols);
            const Matrix &M = *this;
            Matrix output(M.rows, M.cols);
            const size_t size = rows * cols;
            std::transform(data.begin(), data.begin() + size, t.data.begin(), output.data.begin(),
                        [](double a, double b) { return a + b; }); // Perform element-wise addition
            return output;
        }

        void print()
        {
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    std::cout << get(i, j) << ", ";
                }
                std::cout << std::endl;
            }
        }

        T max()
        {
            const int entries = rows * cols;
            return *std::max_element(data.begin(), data.begin() + entries);
        }

        T min()
        {
            const int entries = rows * cols;
            return *std::min_element(data.begin(), data.begin() + entries);
        }
    };

    template <class Types>
    typename Types::Real exploitability(
        typename Types::MatrixReal &row_payoff_matrix,
        typename Types::MatrixReal &col_payoff_matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        const int rows = row_payoff_matrix.rows;
        const int cols = row_payoff_matrix.cols;

        typename Types::Real row_payoff = 0, col_payoff = 0;
        typename Types::VectorReal row_response, col_response;
        row_response.fill(rows, 0);
        col_response.fill(cols, 0);
        for (ActionIndex row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (ActionIndex col_idx = 0; col_idx < cols; ++col_idx)
            {
                const size_t data_idx = row_idx * cols + col_idx;
                const typename Types::Real u = row_payoff_matrix.data[data_idx] * col_strategy[col_idx];
                const typename Types::Real v = col_payoff_matrix.data[data_idx] * row_strategy[row_idx];
                row_payoff += u * row_strategy[row_idx];
                col_payoff += v * col_strategy[col_idx];
                row_response[row_idx] += u;
                col_response[col_idx] += v;
            }
        }

        typename Types::Real row_best_response = *std::max_element(row_response.begin(), row_response.end());
        typename Types::Real col_best_response = *std::max_element(col_response.begin(), col_response.end());

        return (row_best_response - row_payoff) + (col_best_response - col_payoff);
    }
}