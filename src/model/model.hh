#pragma once

#include "../state/state.hh"

template <class _State>
class AbstractModel
{
public:
    struct Types : _State::Types
    {
        using State = _State;
    };
};

/*
Similar to `State`, in that virtually all models will be derived from it.
*/

template <class _State>

class DoubleOracleModel : public AbstractModel<_State>
{
    static_assert(std::derived_from<_State, State<typename _State::Types::TypeList>>);

public:
    struct Inference;
    struct Types : AbstractModel<_State>::Types
    {
        using Inference = DoubleOracleModel::Inference;
    };

    struct Inference
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
    // static_assert(std::derived_from<State, State<typename State::Types::TypeList>>);

public:
    struct Types : DoubleOracleModel<State>::Types
    {
    };

    typename Types::PRNG device;

    MonteCarloModel(typename Types::PRNG &device) : device(device) {}

    void get_inference(
        typename Types::State &state,
        typename Types::Inference &inference)
    {
        const typename Types::Real row_uniform{1 / (typename Types::Real)state.row_actions.size()};
        inference.row_policy.fill(state.row_actions.size(), row_uniform);
        const typename Types::Real col_uniform{1 / (typename Types::Real)state.col_actions.size()};
        inference.col_policy.fill(state.col_actions.size(), col_uniform);

        rollout(state);
        inference.row_value = state.row_payoff;
        inference.col_value = state.col_payoff;
    }

protected:
    void rollout(State &state)
    {
        // model inference in bandits happens in expand(), after get_actions is called
        while (!state.is_terminal)
        {
            const ActionIndex row_idx = device.random_int(state.row_actions.size());
            const ActionIndex col_idx = device.random_int(state.col_actions.size());
            const typename Types::Action row_action = state.row_actions[row_idx];
            const typename Types::Action col_action = state.col_actions[col_idx];
            state.apply_actions(row_action, col_action);
            state.get_actions();
        }
    }
};