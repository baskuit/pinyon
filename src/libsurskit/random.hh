#pragma once

#include <random>
#include <array>
/*
The only behaviour needing explanation is the copy mechanism.
We don't override the default copy constructor. That preserves the progress of the generator as well as the seed.
`copy()` returns one with the same seed but 'restarted'.
*/

class prng
{
    std::mt19937::result_type seed;
    std::mt19937 engine;
    std::uniform_real_distribution<double> uniform_;

public:
    prng() : seed(std::random_device{}()), engine(std::mt19937{seed}) {}
    prng(std::mt19937::result_type seed) : seed(seed), engine(std::mt19937{seed}) {}

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
    // int sample_pdf(double *input, int k)
    // {
    //     double p = this->uniform();
    //     for (int i = 0; i < k; ++i)
    //     {
    //         p -= input[i];
    //         if (p <= 0)
    //         {
    //             return i;
    //         }
    //     }
    //     return 0;
    // }

    template <typename Vector>
    int sample_pdf(Vector &input, int k)
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