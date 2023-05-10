#pragma once
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "random.hh"
#include "rational.hh"
#include "vector.hh"

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

    template <typename Real>
    Real sigmoid (Real x) {
        return 1 / (1 + exp(x));
    }
}

namespace Linear
{

    template <typename T, int size>
    class Matrix
    {
    public:
        std::array<T, size * size> data;
        int rows, cols;
        Matrix(){};
        Matrix(int rows, int cols) : rows(rows), cols(cols) {}

        void fill(int rows, int cols) {
            this->rows = rows;
            this->cols = cols;
        }

        void fill(int rows, int cols, T value)
        {
            this->rows = rows;
            this->cols = cols;
            std::fill(data.begin(), data.begin() + rows * cols, value);
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

    template <typename T>
    class MatrixVector
    {
    public:
        std::vector<T> data;
        int rows, cols;

        MatrixVector(){};
        MatrixVector(int rows, int cols) : data(std::vector<T>(rows * cols)), rows(rows), cols(cols)
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

        MatrixVector operator*(T t)
        {
            const MatrixVector &M = *this;
            MatrixVector output(M.rows, M.cols);
            for (int i = 0; i < rows * cols; ++i)
            {
                output.data[i] = M.data[i] * t;
            }
            return output;
        }
        MatrixVector operator+(T t)
        {
            const MatrixVector &M = *this;
            MatrixVector output(M.rows, M.cols);
            for (int i = 0; i < rows * cols; ++i)
            {
                output.data[i] = M.data[i] + t;
            }
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

    template <class TypeList>
    typename TypeList::Real exploitability(
        typename TypeList::MatrixReal &row_payoff_matrix,
        typename TypeList::MatrixReal &col_payoff_matrix,
        typename TypeList::VectorReal &row_strategy,
        typename TypeList::VectorReal &col_strategy)
    {
        const int rows = row_payoff_matrix.rows;
        const int cols = row_payoff_matrix.cols;

        typename TypeList::Real row_payoff = 0, col_payoff = 0;
        typename TypeList::VectorReal row_response, col_response;
        row_response.fill(rows, 0);
        col_response.fill(cols, 0); // TODO maybe replace this with just a constructor
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                const typename TypeList::Real u = row_payoff_matrix.get(row_idx, col_idx) * col_strategy[col_idx];
                const typename TypeList::Real v = col_payoff_matrix.get(row_idx, col_idx) * row_strategy[row_idx];
                row_payoff += u * row_strategy[row_idx];
                col_payoff += v * col_strategy[col_idx];
                row_response[row_idx] += u;
                col_response[col_idx] += v;
            }
        }

        typename TypeList::Real row_best_response = row_response[0], col_best_response = col_response[0];
        for (int row_idx = 1; row_idx < rows; ++row_idx)
        {
            if (row_response[row_idx] > row_best_response)
            {
                row_best_response = row_response[row_idx];
            }
        }
        for (int col_idx = 1; col_idx < cols; ++col_idx)
        {
            if (col_response[col_idx] > col_best_response)
            {
                col_best_response = col_response[col_idx];
            }
        }

        return (row_best_response - row_payoff) + (col_best_response - col_payoff);
    }
}