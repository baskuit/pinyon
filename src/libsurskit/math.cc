#include "math.hh"

// replace with std:: equiv or something this is too jank
float math::randomProb () {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

// samples a probabilty distribution
int math::sample (float* input, int k) {
    float p = randomProb();
    for (int i = 0; i < k; ++i) {
        p -= input[i];
        if (p <= 0) {
            return i;
        }
    }
    return 0;
}

// equivalent to multiplying the logits by 'power'
void math::powerNorm(int* input, int k, float power, float* output) {
    float sum = 0;
    for (int i = 0; i < k; ++i) {
        float x = std::pow(input[i], power);
        output[i] = x;
        sum += x;
    }
    for (int i = 0; i < k; ++i) {
        output[i] = output[i]/sum;
    }
}