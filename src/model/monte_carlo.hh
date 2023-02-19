#pragma once

#include "model.hh"

template <typename State>
class MonteCarlo : public Model<State>
{
public:
    struct InferenceData : Model<State>::InferenceData
    {
        std::array<double, State::size_> strategy_prior0;
        std::array<double, State::size_> strategy_prior1;
        double value0;
        double value1;
    };

    prng &device;
    // Shadows the base InferenceData type? Also shadows the base last_inference member too, but at no cost?
    InferenceData last_inference;

    MonteCarlo(prng &device) : device(device){};

    // The return type is a comprimise.
    // We would like covariance since each model really should have its own return type.
    // But using new with a pointer would be slow, so we have a storage member in the class that we modify and return always.
    MonteCarlo<State>::InferenceData &inference(State &state, typename MonteCarlo::pair_actions_t &legal_actions)
    {
        for (int row_idx = 0; row_idx < legal_actions.rows; ++row_idx)
        {
            last_inference.strategy_prior0[row_idx] = 1 / (double)legal_actions.rows;
        }
        for (int col_idx = 0; col_idx < legal_actions.cols; ++col_idx)
        {
            last_inference.strategy_prior1[col_idx] = 1 / (double)legal_actions.cols;
        }
        rollout(state);
        last_inference.value0 = state.payoff0;
        last_inference.value1 = state.payoff1;
        return last_inference;
    };

    void rollout(State &state)
    {
        typename MonteCarlo::pair_actions_t legal_actions = state.get_legal_actions();
        while (legal_actions.rows * legal_actions.cols != 0)
        {
            int row_idx = this->device.random_int(legal_actions.rows);
            int col_idx = this->device.random_int(legal_actions.cols);
            typename MonteCarlo::action_t action0 = legal_actions.actions0[row_idx];
            typename MonteCarlo::action_t action1 = legal_actions.actions1[col_idx];
            state.apply_actions(action0, action1);
            state.get_legal_actions(legal_actions);
        }
    }
};

template <typename State>
class MonteCarloWithPolicy : public Model<State>
{
public:
    double p = 1;

    struct InferenceData : Model<State>::InferenceData
    {
        std::array<double, State::size_> strategy_prior0;
        std::array<double, State::size_> strategy_prior1;
        double value0;
        double value1;
    };

    prng &device;
    // Shadows the base InferenceData type? Also shadows the base last_inference member too, but at no cost?
    InferenceData last_inference;

    MonteCarloWithPolicy(prng &device) : device(device){};

    MonteCarloWithPolicy<State>::InferenceData &inference(State &state, typename MonteCarloWithPolicy::pair_actions_t &legal_actions)
    {
        math::power_norm<double, State::size_>(state.strategy0, state.rows, p, last_inference.strategy_prior0);
        math::power_norm<double, State::size_>(state.strategy1, state.cols, p, last_inference.strategy_prior1);
        rollout(state);
        last_inference.value0 = state.payoff0;
        last_inference.value1 = state.payoff1;
        return last_inference;
    };

    void rollout(State &state)
    {
        typename MonteCarloWithPolicy::pair_actions_t legal_actions = state.get_legal_actions();
        while (legal_actions.rows * legal_actions.cols != 0)
        {
            int row_idx = this->device.random_int(legal_actions.rows);
            int col_idx = this->device.random_int(legal_actions.cols);
            typename MonteCarloWithPolicy::action_t action0 = legal_actions.actions0[row_idx];
            typename MonteCarloWithPolicy::action_t action1 = legal_actions.actions1[col_idx];
            state.apply_actions(action0, action1);
            state.get_legal_actions(legal_actions);
        }
    }
};