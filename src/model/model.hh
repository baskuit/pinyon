#pragma once

#include "../state/state.hh"

struct InferenceData {

    float* strategy_prior0;
    float* strategy_prior1;
    float  value_estimate0;
    float  value_estimate1;

    InferenceData (float* strategy_prior0, float* strategy_prior1, float value_estimate0, float value_estimate1) :
    strategy_prior0(strategy_prior0), strategy_prior1(strategy_prior1), value_estimate0(value_estimate0), value_estimate1(value_estimate1) {};
    InferenceData() :
    strategy_prior0(nullptr), strategy_prior1(nullptr), value_estimate0(0.5f), value_estimate1(0.5f) {};
};

class Model {
public:
    Model() {};
    virtual InferenceData inference (State* state) {
        return InferenceData();
    };
    virtual InferenceData inference (State* state, PairActions pair) {
        return InferenceData();
    };
};