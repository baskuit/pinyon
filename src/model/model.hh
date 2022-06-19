#pragma once

#include "../state/state.hh"

template <typename StateType>
class Model {
public:
    typedef StateType state_t;
    typedef typename StateType::action_t action_t;
    typedef typename StateType::pair_actions_t pair_actions_t;
    typedef typename StateType::transition_data_t transition_data_t;

    struct InferenceData {};

    InferenceData inference_;

    virtual InferenceData& inference (StateType& state, pair_actions_t& pair) = 0;
};



// template <int size>
// class SolvedModel : public Model<size> {

//     prng& device;

//     SolvedModel<size> (prng& device) :
//     device(device) {}

//     InferenceData<size> inference (SolvedState<size>& state) {
//         PairActions<size> pair;
//         InferenceData<size> inference_data;
//         state.actions(pair);
//         for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
//             inference_data.strategy_prior0[row_idx] = state.strategy0[row_idx];
//         }
//         for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
//             inference_data.strategy_prior1[col_idx] = state.strategy1[col_idx];
//         }
//         double u = state.payoff;
//         inference_data.value_estimate0 = u;
//         inference_data.value_estimate1 = 1-u;
//         return inference_data;
//     };

//     InferenceData<size> inference (SolvedState<size>& state, PairActions<size>& pair) {
//         InferenceData<size> inference_data;
//         state.actions(pair);
//         for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
//             inference_data.strategy_prior0[row_idx] = state.strategy0[row_idx];
//         }
//         for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
//             inference_data.strategy_prior1[col_idx] = state.strategy1[col_idx];
//         }
//         double u = state.payoff;
//         inference_data.value_estimate0 = u;
//         inference_data.value_estimate1 = 1-u;
//         return inference_data;
//     };
// };