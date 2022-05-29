#include "math.hh"

// equivalent to multiplying the logits by 'power'
void math::power_norm(float* input, int k, float power, float* output) {
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