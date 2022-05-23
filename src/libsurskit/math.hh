#pragma once
#include <math.h>

namespace math {

    // replace with std:: equiv or something this is too jank
    float randomProb ();

    //samples a probabilty distribution
    int sample (float* input, int k);

    void powerNorm(int* input, int k, float power, float* output);
}