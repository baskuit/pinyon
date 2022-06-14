#pragma once

#include "../state/state.hh"

template <int size>
struct InferenceData {

    std::array<double, size> strategy_prior0;
    std::array<double, size> strategy_prior1;
    double  value_estimate0 = .5f;
    double  value_estimate1 = .5f;

    InferenceData () {}
    InferenceData (double value_estimate0, double value_estimate1) :
    strategy_prior0(strategy_prior0), strategy_prior1(strategy_prior1), value_estimate0(value_estimate0), value_estimate1(value_estimate1) {}

    void print () {

        std::cout << "prior0: ";
        for (int i = 0; i < size; ++i) {
            std::cout << strategy_prior0[i] << ' ';
        }
        std::cout << std::endl;
        std::cout << "prior1: ";
        for (int i = 0; i < size; ++i) {
            std::cout << strategy_prior1[i] << ' ';
        }
        std::cout << std::endl;
        std::cout << "v0: " << value_estimate0 << " v1: " << value_estimate1 << std::endl;
    }

};

template <int size>
class Model {
public:
    Model() {};
    //virtual InferenceData inference (State* state, PairActions& pair) = 0;
    virtual InferenceData<size> inference (State<size>& state) = 0;
    virtual InferenceData<size> inference (State<size>& state, PairActions<size>& pair) = 0;
};

template <int size>
class MonteCarlo : public Model<size> {
public:

    prng& device;

    MonteCarlo (prng& device) : device(device) {};

    InferenceData<size> inference (State<size>& state) {
        PairActions<size> pair;
        InferenceData<size> inference_data;
        double u = rollout(state);
        inference_data.value_estimate0 = u;
        inference_data.value_estimate1 = 1-u;
        return inference_data;
    };

    InferenceData<size> inference (State<size>& state, PairActions<size>& pair) {
        InferenceData<size> inference_data;
        state.actions(pair);
        for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
            inference_data.strategy_prior0[row_idx] = 1 / (double) pair.rows;
        }
        for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
            inference_data.strategy_prior1[col_idx] = 1 / (double) pair.cols;
        }
        double u = rollout(state);
        inference_data.value_estimate0 = u;
        inference_data.value_estimate1 = 1-u;
        return inference_data;
    };

    double rollout (State<size>& state) {
        PairActions<size> pair;
        while (pair.rows * pair.cols != 0) {
            int row_idx = this->device.random_int(pair.rows);
            int col_idx = this->device.random_int(pair.cols);
            Action action0 = pair.actions0[row_idx];
            Action action1 = pair.actions1[col_idx];
            state.transition(action0, action1);
            state.actions(pair);
        }
        return state.payoff;
    }

};

template <int size>
class SolvedModel : public Model<size> {

    prng& device;

    SolvedModel<size> (prng& device) :
    device(device) {}

    InferenceData<size> inference (SolvedState<size>& state) {
        PairActions<size> pair;
        InferenceData<size> inference_data;
        state.actions(pair);
        for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
            inference_data.strategy_prior0[row_idx] = state.strategy0[row_idx];
        }
        for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
            inference_data.strategy_prior1[col_idx] = state.strategy1[col_idx];
        }
        double u = state.payoff;
        inference_data.value_estimate0 = u;
        inference_data.value_estimate1 = 1-u;
        return inference_data;
    };

    InferenceData<size> inference (SolvedState<size>& state, PairActions<size>& pair) {
        InferenceData<size> inference_data;
        state.actions(pair);
        for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
            inference_data.strategy_prior0[row_idx] = state.strategy0[row_idx];
        }
        for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
            inference_data.strategy_prior1[col_idx] = state.strategy1[col_idx];
        }
        double u = state.payoff;
        inference_data.value_estimate0 = u;
        inference_data.value_estimate1 = 1-u;
        return inference_data;
    };
};