#pragma once
#include <math.h>
#include <assert.h>

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