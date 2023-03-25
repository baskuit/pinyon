#pragma once
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <algorithm>

#include "random.hh"
#include "rational.hh"

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

    template <typename Vector>
    void print(Vector &input, int length)
    {
        for (int i = 0; i < length; ++i)
        {
            std::cout << input[i] << ", ";
        }
        std::cout << std::endl;
    }
}

namespace Linear
{

    template <typename T, int size>
    class Matrix
    {
    public:
        std::array<std::array<T, size>, size> data;
        int rows, cols;
        Matrix(){};
        Matrix(int rows, int cols) : rows(rows), cols(cols) {}
        Matrix(const Matrix &M)
        {
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    data[i][j] = M.data[i][j];
                }
            }
            rows = M.rows;
            cols = M.cols;
        }
        Matrix(const std::array<T, size> array, int length)
        {
            // gives row matrix
            this->data[0] = array;
            this->rows = 1;
            this->cols = length;
        }

        T &get(int i, int j)
        {
            return data[i][j];
        }
        void print()
        {
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    std::cout << data[i][j] << ", ";
                }
                std::cout << std::endl;
            }
        }

        Matrix operator*(Matrix &N)
        {
            const Matrix &M = *this;
            assert(M.cols == N.rows);
            Matrix output(M.rows, N.cols);
            for (int i = 0; i < output.rows; ++i)
            {
                for (int j = 0; j < output.cols; ++j)
                {
                    output.data[i][j] = 0;
                    for (int k = 0; k < M.cols; ++k)
                    {
                        output.data[i][j] += M.data[i][k] * N.data[k][j];
                    }
                }
            }
            return output;
        }
        Matrix operator*(T t)
        {
            const Matrix &M = *this;
            Matrix output(M.rows, M.cols);
            for (int i = 0; i < output.rows; ++i)
            {
                for (int j = 0; j < output.cols; ++j)
                {
                    output.data[i][j] = M.data[i][j] * t;
                }
            }
            return output;
        }
        Matrix operator+(T t)
        {
            const Matrix &M = *this;
            Matrix output(M.rows, M.cols);
            for (int i = 0; i < output.rows; ++i)
            {
                for (int j = 0; j < output.cols; ++j)
                {
                    output.data[i][j] = M.data[i][j] + t;
                }
            }
            return output;
        }
        Matrix transpose()
        {
            Matrix t(cols, rows);
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    t.data[j][i] = data[i][j];
                }
            }
            return t;
        }
        T max()
        {
            T x = data[0][0];
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    x = std::max(data[i][j], x);
                }
            }
            return x;
        }
        T min()
        {
            T x = data[0][0];
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    x = std::min(data[i][j], x);
                }
            }
            return x;
        }
    };

    template <class TypeList>
    typename TypeList::Real exploitability(
        typename TypeList::MatrixReal &row_payoff_matrix,
        typename TypeList::MatrixReal &col_payoff_matrix,
        typename TypeList::VectorReal &row_strategy,
        typename TypeList::VectorReal &col_strategy,
        typename TypeList::Real payoff_sum = 1)
    {
        typename TypeList::MatrixReal row_strategy_matrix(row_strategy, row_payoff_matrix.rows);
        typename TypeList::MatrixReal col_strategy_matrix(col_strategy, col_payoff_matrix.cols);
        col_strategy_matrix = col_strategy_matrix.transpose();
        typename TypeList::MatrixReal row_best_response = row_payoff_matrix * col_strategy_matrix;
        typename TypeList::MatrixReal col_best_response = row_strategy_matrix * col_payoff_matrix;
        typename TypeList::Real max_row = row_best_response.max();
        typename TypeList::Real max_col = col_best_response.max();
        return max_row + max_col - payoff_sum;
    }
}