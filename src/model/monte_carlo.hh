#pragma once

#include "model.hh"

template <typename StateType>
class MonteCarlo : public Model<StateType> {
public:

    struct InferenceData : Model<StateType>::InferenceData {
        std::array<double, StateType::size_> strategy_prior0;
        std::array<double, StateType::size_> strategy_prior1;
        double value_estimate0 = 0.5;
        double value_estimate1 = 0.5;
    };

    prng& device;
    InferenceData inference_;

    MonteCarlo (prng& device) : device(device) {};

    MonteCarlo<StateType>::InferenceData& inference (StateType& state, typename MonteCarlo::pair_actions_t& pair) {
        for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
            inference_.strategy_prior0[row_idx] = 1 / (double) pair.rows;
        }
        for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
            inference_.strategy_prior1[col_idx] = 1 / (double) pair.cols;
        }
        double u = rollout(state);
        inference_.value_estimate0 = u;
        inference_.value_estimate1 = 1-u;
        return inference_;
    };

    double rollout (StateType& state) {
        typename MonteCarlo::pair_actions_t pair = state.actions();
        while (pair.rows * pair.cols != 0) {
            int row_idx = this->device.random_int(pair.rows);
            int col_idx = this->device.random_int(pair.cols);
            typename MonteCarlo::action_t action0 = pair.actions0[row_idx];
            typename MonteCarlo::action_t action1 = pair.actions1[col_idx];
            state.transition(action0, action1);
            pair = state.actions();
        }
        return state.payoff0;
    }

};