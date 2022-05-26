#pragma once

#include "../state/state.hh"
// container for Monte Carlo estimation and neural nets

struct InferenceData {
    InferenceData (float value_estimate0, float value_estimate1, float* strategy_prior0, float* strategy_prior1) :
    value_estimate0(value_estimate0), value_estimate1(value_estimate1), strategy_prior0(strategy_prior0), strategy_prior1(strategy_prior1) {};
    float  value_estimate0;
    float  value_estimate1;
    float* strategy_prior0;
    float* strategy_prior1;
};

class Model {
public:
    Model() {};
    virtual InferenceData inference (State* state) {
        InferenceData x = {.5f,.5f,nullptr, nullptr};
        return x;
    };
};