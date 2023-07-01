#pragma once

#include <gmpxx.h>
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
    std::uniform_int_distribution<uint64_t> uniform_64_;

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

    // Uniform random in (0, 1)
    double uniform()
    {
        return uniform_(engine);
    }

    // Random integer in [0, n)
    int random_int(int n)
    {
        return uniform_64_(engine) % n;
    }

    uint64_t uniform_64()
    {
        return uniform_64_(engine);
    }

    mpq_class uniform_mpq(const size_t q)
    {
        const size_t den = random_int(q) + 1;
        return mpq_class{random_int(den + 1), den};
    }

    template <typename Vector>
    int sample_pdf(Vector &input, int k)
    {
        double p = uniform();
        for (int i = 0; i < k; ++i)
        {
            p -= input[i].unwrap();
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    template <typename Vector>
    int sample_pdf(Vector &input)
    {
        double p = uniform();
        for (int i = 0; i < input.size(); ++i)
        {
            p -= input[i];
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    void discard(size_t n)
    {
        engine.discard(n);
    }
};
