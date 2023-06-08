#pragma once

#include <random>
#include <array>

/*
The only behaviour needing explanation is the copy mechanism.
We don't override the default copy constructor. That preserves the progress of the generator as well as the seed.
`copy()` returns one with the same seed but 'restarted'.
*/

struct NullSeed {};

class prng
{
    std::mt19937::result_type seed;
    std::mt19937 engine;
    std::uniform_real_distribution<double> uniform_;
    std::uniform_int_distribution<uint32_t> uniform_32_;

public:

    template <typename Seed>
    Seed new_seed ();

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

    // Random integer in [0, n]
    int random_int(int n)
    {
        return int(this->uniform() * n);
    }

    uint64_t uniform_64() {
        return static_cast<uint64_t>(uniform_32_(engine)) << 32 | uniform_32_(engine);
    }

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

    void discard (int n) {
        engine.discard(n);
    }
};

template <>
uint64_t prng::new_seed<uint64_t>()
{
    return uniform_64();
}

template <>
NullSeed prng::new_seed<NullSeed>()
{
    return NullSeed{};
}