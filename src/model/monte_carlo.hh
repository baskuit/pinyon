#pragma once

#include "model.hh"

template <typename State>
class MonteCarlo : public Model<State> {
public:

    struct InferenceData : Model<State>::InferenceData {
        std::array<double, State::size_> strategy_prior0;
        std::array<double, State::size_> strategy_prior1;
        double value_estimate0;
        double value_estimate1;
    };

    prng& device;
    // Shadows the base InferenceData type? Also shadows the base inference_ member too, but at no cost?
    InferenceData inference_;

    MonteCarlo (prng& device) : device(device) {};

    // The return type is a comprimise.
    // We would like covariance since each model really should have its own return type.
    // But using new with a pointer would be slow, so we have a storage member in the class that we modify and return always.
    MonteCarlo<State>::InferenceData& inference (State& state, typename MonteCarlo::pair_actions_t& pair) {
        for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
            inference_.strategy_prior0[row_idx] = 1 / (double) pair.rows;
        }
        for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
            inference_.strategy_prior1[col_idx] = 1 / (double) pair.cols;
        }
        rollout(state);
        inference_.value_estimate0 = state.payoff0;
        inference_.value_estimate1 = state.payoff1;
        return inference_;
    };

    void rollout (State& state) {
        typename MonteCarlo::pair_actions_t pair = state.actions();
        while (pair.rows * pair.cols != 0) {
            int row_idx = this->device.random_int(pair.rows);
            int col_idx = this->device.random_int(pair.cols);
            typename MonteCarlo::action_t action0 = pair.actions0[row_idx];
            typename MonteCarlo::action_t action1 = pair.actions1[col_idx];
            state.transition(action0, action1);
            state.actions(pair);
        }
    }

};