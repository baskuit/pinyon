#pragma once
#include <math.h>
#include <assert.h>
#include <stdexcept>

#include "random.hh"
#include "rational.hh"

namespace math {

    void power_norm(double* input, int k, double power, double* output);

    template <typename T, int size>
    void power_norm(std::array<T, size> input, int k, double power, std::array<T, size>& output) {
        double sum = 0;
        for (int i = 0; i < k; ++i) {
            double x = std::pow(input[i], power);
            output[i] = x;
            sum += x;
        }
        for (int i = 0; i < k; ++i) {
            output[i] = output[i]/sum;
        }
    }

    template <typename T, typename U, int size>
    void power_norm(std::array<T, size> input, int k, double power, std::array<U, size>& output) {
        double sum = 0;
        for (int i = 0; i < k; ++i) {
            U x = std::pow(static_cast<U>(input[i]), power);
            output[i] = x;
            sum += x;
        }
        for (int i = 0; i < k; ++i) {
            output[i] = output[i]/sum;
        }
    }

}

namespace Linear {

template <typename T, int size>
struct Vector {
    int length = 0;
    std::array<T, size> data;

    template <int k>
    Vector<T, size> (std::array<T, k>& array) {
        length = std::min(size, k);
        for (int i = 0; i < length; ++i) {
            data[i] = array[i];
        }
    }

    Vector (int length) : length(length) {
        for (int i = 0; i < length; ++i) {
            data[i] = Rational(0, 1);
        }
    }
};

template <typename T, int size>
struct Matrix {
    int rows = 0;
    int cols = 0;
    Matrix (int rows, int cols) :
    rows(rows), cols(cols) {}

    Matrix<T,size> operator* (const Matrix<T, size>& M);
    virtual T get (int row_idx, int col_idx) = 0;
    virtual void set (int row_idx, int col_idx, T value) = 0;
};

template <typename T, int size>
struct Matrix2D : Matrix<T, size> {
    std::array<std::array<T, size>, size> data;

    Matrix2D<T, size> (int rows, int cols) :
    Matrix<T, size>(rows, cols) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = Rational(1, 2);
            }
        }
    }

    Matrix2D<T, size> (int rows, int cols, Rational value) :
    Matrix<T, size>(rows, cols) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = value;
            }
        }
    }

    Matrix2D<T, size> operator* (const Matrix2D<T, size>& M) {
        assert(this->cols = M.rows);
        Matrix2D<T, size> product(this->rows, M.cols, Rational(0, 1));
        for (int i = 0; i < this->rows; ++i) {
            for (int j = 0; j < M.cols; ++j) {
                for (int k = 0; k < this->cols; ++k) {
                    product.data[i][j] = product.data[i][j] + (this->data[i][k] * M.data[k][j]);
                }
            }
        }
        return product;
    }

    T get (int row_idx, int col_idx) {
        return data[row_idx][col_idx];
    }

    void set (int row_idx, int col_idx, T value) {
        data[row_idx][col_idx] = value;
    }

    void print () {
        for (int i = 0; i < this->rows; ++i) {
            std::cout << "[";
            for (int j = 0; j < this->cols; ++j) {
                std::cout << get(i, j) << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }


};



};