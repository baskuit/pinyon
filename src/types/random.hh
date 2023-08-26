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

    std::mt19937::result_type get_seed() const
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

    template <template <typename...> typename Vector, template <typename> typename Wrapper, typename T>
    int sample_pdf(const Vector<Wrapper<T>> &input, int k)
    {
        double p = uniform();
        for (int i = 0; i < k; ++i)
        {
            p -= static_cast<double>(input[i]);
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    template <typename Vector>
    int sample_pdf(const Vector &input)
    {
        double p = uniform();
        for (int i = 0; i < input.size(); ++i)
        {
            p -= static_cast<double>(input[i]);
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

class xor_shift
{
    uint64_t seed;
    uint64_t state;

public:
    xor_shift() : seed(std::random_device{}()), state(seed) {}
    xor_shift(uint64_t seed) : seed(seed), state(seed) {}

    uint64_t get_seed() const
    {
        return seed;
    }

    // Uniform random in (0, 1)
    double uniform()
    {
        return static_cast<double>(xorshift()) / static_cast<double>(UINT64_MAX);
    }

    // Random integer in [0, n)
    int random_int(int n)
    {
        return xorshift() % n;
    }

    uint64_t uniform_64()
    {
        return xorshift();
    }

    template <template <typename...> typename Vector, template <typename> typename Wrapper, typename T>
    int sample_pdf(const Vector<Wrapper<T>> &input, int k)
    {
        double p = uniform();
        for (int i = 0; i < k; ++i)
        {
            p -= static_cast<double>(input[i]);
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    template <typename Vector>
    int sample_pdf(const Vector &input)
    {
        double p = uniform();
        for (int i = 0; i < input.size(); ++i)
        {
            p -= static_cast<double>(input[i]);
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    void discard(size_t n)
    {
        for (size_t i = 0; i < n; ++i)
        {
            xorshift();
        }
    }

private:
    uint64_t xorshift()
    {
        state ^= (state << 21);
        state ^= (state >> 35);
        state ^= (state << 4);
        return state;
    }
};
