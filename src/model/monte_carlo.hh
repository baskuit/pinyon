#pragma once

#include "model.hh"

class MonteCarlo : public Model {
public:

    prng* device;

    MonteCarlo (prng* device) : device(device) {};

    void inference (State* state, PairActions& pair, InferenceData& inference_data) {
        float u = rollout(state, pair);
        for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
            inference_data.strategy_prior0[row_idx] = 1 / (float) pair.rows;
        }
        for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
            inference_data.strategy_prior1[col_idx] = 1 / (float) pair.cols;
        }
        inference_data.value_estimate0 = u;
        inference_data.value_estimate1 = 1-u;
    };

private:
    float rollout (State* state, PairActions& pair) {
        
        state->actions(pair);
        while (pair.rows > 0 && pair.cols > 0) {

            const int row_idx = device->random_int(pair.rows);
            const int col_idx = device->random_int(pair.cols);
            
            const Action action0 = pair.actions0[row_idx];
            const Action action1 = pair.actions1[col_idx];
            
            state->transition(action0, action1);
            state->actions(pair);
        }
        return state->payoff;
    };


    // InferenceData inference (State* state){
    //     PairActions* pair = state->actions();
    //     float value_estimate0 = state->rollout();
    //     float value_estimate1 = 1-value_estimate0;
    //     float strategy_prior0[pair->rows] = {1.f/pair->rows};
    //     float strategy_prior1[pair->cols] = {1.f/pair->cols};
    //     InferenceData data =   {strategy_prior0, 
    //                             strategy_prior1,
    //                             value_estimate0, 
    //                             value_estimate1};
    //     return data;
    // };

    // InferenceData inference (State* state, PairActions* pair){
    //     float value_estimate0 = state->rollout();
    //     float value_estimate1 = 1-value_estimate0;
    //     float strategy_prior0[pair->rows] = {1.f/pair->rows};
    //     float strategy_prior1[pair->cols] = {1.f/pair->cols};
    //     InferenceData data =   {strategy_prior0, 
    //                             strategy_prior1,
    //                             value_estimate0, 
    //                             value_estimate1};
    //     return data;
    // };
};