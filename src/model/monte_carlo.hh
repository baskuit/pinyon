#pragma once

#include "model.hh"

template <typename State>
class MonteCarlo : public Model<State>
{
public:
    struct InferenceData : Model<State>::InferenceData
    {
        typename MonteCarlo::VectorDouble strategy_prior0;
        typename MonteCarlo::VectorDouble strategy_prior1;
        // double value0;
        // double value1;
    };

    prng &device;
    InferenceData inference_data;

    MonteCarlo(prng &device) : device(device){};

    void inference(State &state)
    {
        for (int row_idx = 0; row_idx < state.pair_actions.rows; ++row_idx)
        {
            MonteCarlo::inference_data.strategy_prior0[row_idx] = 1 / (double)state.pair_actions.rows;
        }
        for (int col_idx = 0; col_idx < state.pair_actions.cols; ++col_idx)
        {
            MonteCarlo::inference_data.strategy_prior1[col_idx] = 1 / (double)state.pair_actions.cols;
        }
        rollout(state);
        MonteCarlo::inference_data.value0 = state.row_payoff;
        MonteCarlo::inference_data.value1 = state.col_payoff;
    };

    void rollout(State &state)
    {
        // state.get_legal_actions();
        // In all bandit algo's, we always get actions before applying inference.
        while (!state.terminal)
        {
            int row_idx = this->device.random_int(state.pair_actions.rows);
            int col_idx = this->device.random_int(state.pair_actions.cols);
            typename MonteCarlo::PlayerAction action0 = state.pair_actions.row_actions[row_idx];
            typename MonteCarlo::PlayerAction action1 = state.pair_actions.col_actions[col_idx];
            state.apply_actions(action0, action1);
            state.get_player_actions();
        }
    }
};

// template <typename State>
// class MonteCarloWithPolicy : public Model<State>
// {
// public:
//     double p = 1;

//     struct InferenceData : Model<State>::InferenceData
//     {
//         std::array<double, State::_size> strategy_prior0;
//         std::array<double, State::_size> strategy_prior1;
//         double value0;
//         double value1;
//     };

//     prng &device;
//     // Shadows the base InferenceData type? Also shadows the base last_inference member too, but at no cost?
//     InferenceData last_inference;

//     MonteCarloWithPolicy(prng &device) : device(device){};

//     MonteCarloWithPolicy<State>::InferenceData &inference(State &state, typename MonteCarloWithPolicy::pair_actions_t &legal_actions)
//     {
//         math::power_norm<double, State::_size>(state.strategy0, state.rows, p, last_inference.strategy_prior0);
//         math::power_norm<double, State::_size>(state.strategy1, state.cols, p, last_inference.strategy_prior1);
//         rollout(state);
//         last_inference.value0 = state.payoff0;
//         last_inference.value1 = state.payoff1;
//         return last_inference;
//     };

//     void rollout(State &state)
//     {
//         typename MonteCarloWithPolicy::pair_actions_t legal_actions = state.get_legal_actions();
//         while (legal_actions.rows * legal_actions.cols != 0)
//         {
//             int row_idx = this->device.random_int(legal_actions.rows);
//             int col_idx = this->device.random_int(legal_actions.cols);
//             typename MonteCarloWithPolicy::action_t action0 = legal_actions.actions0[row_idx];
//             typename MonteCarloWithPolicy::action_t action1 = legal_actions.actions1[col_idx];
//             state.apply_actions(action0, action1);
//             state.get_legal_actions(legal_actions);
//         }
//     }
// };