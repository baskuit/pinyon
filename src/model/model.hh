#pragma once

#include "../state/state.hh"

template <int size>
struct InferenceData {

    std::array<float, size> strategy_prior0;
    std::array<float, size> strategy_prior1;
    float  value_estimate0 = .5f;
    float  value_estimate1 = .5f;

    InferenceData () {}
    InferenceData (float value_estimate0, float value_estimate1) :
    strategy_prior0(strategy_prior0), strategy_prior1(strategy_prior1), value_estimate0(value_estimate0), value_estimate1(value_estimate1) {}

};

template <int size>
class Model {
public:
    Model() {};
    //virtual InferenceData inference (State* state, PairActions& pair) = 0;
    virtual InferenceData<size> inference (State<size>& state) = 0;
};

template <int size>
class MonteCarlo : public Model<size> {
public:

    prng& device;

    MonteCarlo (prng& device) : device(device) {};

    InferenceData<size> inference (State<size>& state) {
        PairActions<size> pair;
        InferenceData<size> inference_data;
        float u = state.rollout();
        // fill w/ uniform /.... or dont
        inference_data.value_estimate0 = u;
        inference_data.value_estimate1 = 1-u;
        return inference_data;
    };
};