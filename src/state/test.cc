#include <vector>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <cmath>
#include <algorithm>

const double eta = .01 / 9;

template <typename Real, typename VectorReal>
inline void softmax(
    VectorReal &forecast,
    VectorReal &gains,
    const size_t k,
    Real eta)
{
    Real l1_norm = 0;
    std::transform(gains.begin(), gains.begin() + k, forecast.begin(),
                   [&l1_norm, eta](Real logit)
                   {
                       const Real y(std::exp((logit * eta)));
                       l1_norm += y;
                       return y;
                   });
    std::transform(forecast.begin(), forecast.begin() + k, forecast.begin(),
                   [l1_norm](Real probability)
                   {
                       return probability / l1_norm;
                   });
};

template <typename Real, typename VectorReal>
inline void softmax2(
    VectorReal &forecast,
    VectorReal &gains,
    const int k,
    Real eta)
{
    Real sum{0};
    for (int i = 0; i < k; ++i)
    {
        const Real y(std::exp((gains[i] * eta)));
        forecast[i] = y;
        sum += y;
    }
    for (int i = 0; i < k; ++i)
    {
        forecast[i] /= sum;
    }
};

int main()
{

    std::vector<double> gains;
    gains.resize(9);

    std::vector<double> forecast;
    forecast.resize(9);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < 1 << 20; ++i)
    {
        softmax2(forecast, gains, 9, eta);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << duration << std::endl;
}