#pragma once

#include "../model/model.hh"

class MonteCarlo : public Model {
public:
    MonteCarlo();
    InferenceData inference (State* state){
        
        float value_estimate0 = state->rollout();
        float value_estimate1 = 1-value_estimate0;
        float strategy_prior0[state->rows] = {1.f/state->rows};
        float strategy_prior1[state->cols] = {1.f/state->cols};
        InferenceData data = {  value_estimate0, 
                                value_estimate1, 
                                strategy_prior0, 
                                strategy_prior1};
        return data;
    };
};