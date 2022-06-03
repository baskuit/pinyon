#pragma once

#include "model.hh"

class MonteCarlo : public Model {
public:
    MonteCarlo() {};
    InferenceData inference (State* state){
        PairActions* pair = state->actions();
        float value_estimate0 = state->rollout();
        float value_estimate1 = 1-value_estimate0;
        float strategy_prior0[pair.rows] = {1.f/pair.rows};
        float strategy_prior1[pair.cols] = {1.f/pair.cols};
        InferenceData data =   {strategy_prior0, 
                                strategy_prior1,
                                value_estimate0, 
                                value_estimate1};
        return data;
    };

    InferenceData inference (State* state, PairActions pair){
        float value_estimate0 = state->rollout();
        float value_estimate1 = 1-value_estimate0;
        float strategy_prior0[pair.rows] = {1.f/pair.rows};
        float strategy_prior1[pair.cols] = {1.f/pair.cols};
        InferenceData data =   {strategy_prior0, 
                                strategy_prior1,
                                value_estimate0, 
                                value_estimate1};
        return data;
    };
};