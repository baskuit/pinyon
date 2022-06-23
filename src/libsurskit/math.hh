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
    };

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
    };



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
};

template <typename T, int size>
struct Matrix {
    int rows = 0;
    int cols = 0;
    Matrix (int rows, int cols) :
    rows(rows), cols(cols) {}

    virtual Matrix<T,size> operator* (const Matrix<T, size> M) = 0;
};

template <typename T, int size>
struct Matrix2D : Matrix<T, size> {
    std::array<std::array<T, size>, size> data;
};

};