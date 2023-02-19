#pragma once

#include <random>
#include <array>

class prng
{
    std::mt19937::result_type seed;
    std::mt19937 engine;
    std::uniform_real_distribution<double> uniform_;

public:
    prng() : seed(std::random_device{}()), engine(std::mt19937{seed}) {}
    prng(std::mt19937::result_type seed) : seed(seed), engine(std::mt19937{seed}) {}

    // Same device and seed, but 'restarted'
    // Default copy keeps progress
    prng copy()
    {
        return prng(seed);
    }

    std::mt19937::result_type get_seed()
    {
        return seed;
    }

    double uniform()
    {
        return uniform_(engine);
    }

    int random_int(int n)
    {
        return int(this->uniform() * n);
    }

    // samples an index from a probability distribution
    int sample_pdf(double *input, int k)
    {
        double p = this->uniform();
        for (int i = 0; i < k; ++i)
        {
            p -= input[i];
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    template <typename T, int size>
    int sample_pdf(std::array<T, size> &input, int k)
    {
        double p = uniform();
        for (int i = 0; i < k; ++i)
        {
            p -= input[i];
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }
};