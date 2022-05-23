#pragma once

#include "../state/state.hh"
// container for Monte Carlo estimation and neural nets

struct InferenceData {
    float  value_estimate0;
    float  value_estimate1;
    float* strategy_prior0;
    float* strategy_prior1;
};

class Model {
public:
    Model();
    InferenceData inference (State* state);
};