    #include "libsurskit/random.hh"

    #include <cmath>
    
    int dbf(prng &device, int depth)
    {
        return (depth - 1) * (depth >= 0);
    }
    int af(prng &device, int n_actions)
    {
        return n_actions;
    }
    int pbf(prng &device, int payoff_bias)
    {
        const int bias = 1;
        return payoff_bias + device.random_int(2 * bias + 1) - bias;
    }

    template <int n, float p=1>
    int slip_pow(prng &device, int depth)
    {
        double x = pow(device.uniform(), p);
        return (depth - n * x) * (depth >= 0);
    }

    template <int num, int den>
    int biased_payoff(prng &device, int payoff_bias)
    {
        return payoff_bias + device.random_int(2 * den + 1) - den + num;
    }