#include "math.hh"

// equivalent to multiplying the logits by 'power'
void math::power_norm(double* input, int k, double power, double* output) {
    double sum = 0;
    for (int i = 0; i < k; ++i) {
        double x = std::pow(input[i], power);
        output[i] = x;
        sum += x;
    }
    for (int i = 0; i < k; ++i) {
        output[i] = output[i]/sum;
    }
}