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
    //virtual InferenceData inference (State* state) = 0;
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

// private:
//     float rollout (State* state, PairActions& pair) {
        
//         state->actions(pair);
//         while (pair.rows > 0 && pair.cols > 0) {

//             const int row_idx = device.random_int(pair.rows);
//             const int col_idx = device.random_int(pair.cols);
            
//             const Action action0 = pair.actions0[row_idx];
//             const Action action1 = pair.actions1[col_idx];
            
//             state->transition(action0, action1);
//             state->actions(pair);
//         }
//         return state->payoff;
//     };


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
// };