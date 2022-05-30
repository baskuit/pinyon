#include <random>

class prng {
    std::mt19937::result_type seed;
    std::mt19937 engine;
    std::uniform_real_distribution<float> uniform_;
    
public:

    prng () {
        seed = std::random_device{}();
        engine = std::mt19937 {seed};
    }
    prng (std::mt19937::result_type seed) : seed(seed) {
        engine.seed(seed);
    }
    prng (prng const& device) {
        seed = device.seed;
        engine = std::mt19937 {seed};
    }
    // TODO need assigment operator. see undesired behavior of prng_copy_test2

    std::mt19937::result_type get_seed () {
        return seed;
    }

    float uniform () {
        return uniform_(engine);
    }

    int random_int (int n) {
        return int (this->uniform() * n);
    }

    // samples a probabilty distribution
    int sample_pdf (float* input, int k) {
        float p = this->uniform();
        for (int i = 0; i < k; ++i) {
            p -= input[i];
            if (p <= 0) {
                return i;
            }
        }
        return 0;
    }
};