#pragma once

#include "../state/state.hh"

struct InferenceData {

    float* strategy_prior0 = nullptr;
    float* strategy_prior1 = nullptr;
    float  value_estimate0 = .5f;
    float  value_estimate1 = .5f;

    InferenceData (float* strategy_prior0, float* strategy_prior1, float value_estimate0, float value_estimate1) :
    strategy_prior0(strategy_prior0), strategy_prior1(strategy_prior1), value_estimate0(value_estimate0), value_estimate1(value_estimate1) {}
    InferenceData (float* strategy_prior0, float* strategy_prior1) :
    strategy_prior0(strategy_prior0), strategy_prior1(strategy_prior1) {}
    InferenceData() {}
};

class Model {
public:

    Model() {};
    //virtual InferenceData inference (State* state) = 0;
    //virtual InferenceData inference (State* state, PairActions& pair) = 0;
    virtual void inference (State* state, PairActions& pair, InferenceData& inference_data) = 0;
};