#pragma once

#include "../state/state.hh"

template <class _State>
class AbstractModel
{
public:
    struct Types : _State::Types {
        using State = _State;
    };
    struct Inference
    {
    };
};

template <class State>
// requires std::derived_from<State, AbstractState>;
class DualPolicyValueModel : public AbstractModel<State>
{
public:
    struct Inference;
    struct Types : State::Types
    {
        using Inference = DualPolicyValueModel::Inference;
    };

    struct Inference
    {
        typename Types::Real row_value;
        typename Types::Real col_value;
        typename Types::VectorReal row_policy;
        typename Types::VectorReal col_policy;
    };

    void get_inference(
        State &state,
        typename Types::Inference &inference);
};

template <class State>
// requires std::derived_from<State, AbstractState>;
class MonteCarloModel : public DualPolicyValueModel<State>
{
public:
    struct Types : DualPolicyValueModel<State>::Types
    {
    };

    prng& device;

    MonteCarloModel (prng &device) : device(device) {}

    void get_inference(
        State &state,
        typename Types::Inference &inference)
    {
        const typename Types::Real row_uniform = 1 / (typename Types::Real) state.actions.rows;
        for (int i = 0; i < state.actions.rows; ++i) {
            inference.row_policy[i] = row_uniform;
        }
        const typename Types::Real col_uniform = 1 / (typename Types::Real) state.actions.cols;
        for (int j = 0; j < state.actions.cols; ++j) {
            inference.col_policy[j] = col_uniform;
        }
        this->rollout(state);
        inference.row_value = state.row_payoff;
        inference.col_value = state.col_payoff;
    }
private:
    void rollout (State &state) {
        while (!state.is_terminal) {
            const int row_idx = this->device.random_int(state.actions.rows);
            const int col_idx = this->device.random_int(state.actions.cols);
            const typename Types::Action row_action = state.actions.row_actions[row_idx];
            const typename Types::Action col_action = state.actions.col_actions[col_idx];
            state.apply_actions(row_action, col_action);
            state.get_actions();
        }
    }
};

/*
MonteCarlo model that uses a priori solutions to simulate expert inference 
*/

template <class State>
// requires std::derived_from<State, AbstractState>;
// static_assert(1==1);
class SolvedMonteCarloModel : public DualPolicyValueModel<State>
{
public:
    struct Types : DualPolicyValueModel<State>::Types
    {
    };

    prng& device;
    typename Types::Real power = 1;

    SolvedMonteCarloModel (prng &device) : device(device) {}

    void get_inference(
        State &state,
        typename Types::Inference &inference)
    {
        math::power_norm(state.row_strategy, state.actions.rows, this->power, inference.row_strategy);
        math::power_norm(state.col_strategy, state.actions.cols, this->power, inference.col_strategy);
        this->rollout(state);
        inference.row_value = state.row_payoff;
        inference.col_value = state.col_payoff;
    }
private:
    void rollout (State &state) {
        while (!state.is_terminal) {
            const int row_idx = this->device.random_int(state.actions.rows);
            const int col_idx = this->device.random_int(state.actions.cols);
            const typename Types::Action row_action = state.actions.row_actions[row_idx];
            const typename Types::Action col_action = state.actions.col_actions[col_idx];
            state.apply_actions(row_action, col_action);
            state.get_actions();
        }
    }
};