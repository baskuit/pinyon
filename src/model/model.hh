#pragma once

#include "state/state.hh"

template <class _State>
class AbstractModel
{
public:
    struct Inference;
    struct Types : _State::Types
    {
        using State = _State;
        using Inference = AbstractModel::Inference;
    };
    struct Inference
    {
    };

    void get_inference(
        typename Types::State &state,
        typename Types::Inference &inference);
};

/*
Similar to `State`, in that virtually all models will be derived from it.
*/

template <class State>

class DoubleOracleModel : public AbstractModel<State>
{
    static_assert(std::derived_from<State, AbstractState<typename State::Types::TypeList>>);

public:
    struct Inference;
    struct Types : AbstractModel<State>::Types
    {
        using Inference = DoubleOracleModel::Inference;
    };

    struct Inference : AbstractModel<State>::Inference
    {
        typename Types::Real row_value;
        typename Types::Real col_value;
        typename Types::VectorReal row_policy;
        typename Types::VectorReal col_policy;
    };
};

/*
Universal model.
*/

template <class State>
class MonteCarloModel : public DoubleOracleModel<State>
{
    static_assert(std::derived_from<State, AbstractState<typename State::Types::TypeList>>);

public:
    struct Types : DoubleOracleModel<State>::Types
    {
    };

    prng device;

    MonteCarloModel(prng &device) : device(device.get_seed()) {}

    void get_inference(
        typename Types::State &state,
        typename Types::Inference &inference)
    {
        const typename Types::Real row_uniform = 1 / (typename Types::Real)state.actions.rows;
        for (int i = 0; i < state.actions.rows; ++i)
        {
            inference.row_policy[i] = row_uniform;
        }
        const typename Types::Real col_uniform = 1 / (typename Types::Real)state.actions.cols;
        for (int j = 0; j < state.actions.cols; ++j)
        {
            inference.col_policy[j] = col_uniform;
        }
        this->rollout(state);
        inference.row_value = state.row_payoff;
        inference.col_value = state.col_payoff;
    }

protected:
    void rollout(State &state)
    {
        while (!state.is_terminal)
        {
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
class SolvedMonteCarloModel : public MonteCarloModel<State>
{   
    static_assert(std::derived_from<State, SolvedState<typename State::Types::TypeList>>);

public:
    struct Types : DoubleOracleModel<State>::Types
    {
    };

    typename Types::Real power = 1;
    typename Types::Real epsilon = .005; // unused

    SolvedMonteCarloModel(prng &device) : MonteCarloModel<State>(device) {}

    SolvedMonteCarloModel(prng &device, typename Types::Real power, typename Types::Real epsilon) : 
        MonteCarloModel<State>(device), power(power), epsilon(epsilon) {}

    void get_inference(
        typename Types::State &state,
        typename Types::Inference &inference)
    {
        math::power_norm(state.row_strategy, state.actions.rows, power, inference.row_policy);
        math::power_norm(state.col_strategy, state.actions.cols, power, inference.col_policy);
        this->rollout(state);
        inference.row_value = state.row_payoff;
        inference.col_value = state.col_payoff;
    }
};