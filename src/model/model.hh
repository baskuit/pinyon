#pragma once

#include "../state/state.hh"

class AbstractModel
{
public:
    struct Inference
    {
    };
};

template <class State>
// requires std::derived_from<State, AbstractState>;
class PolicyValueModel : public AbstractModel
{
public:
    struct Inference;
    struct Types : public State::Types
    {
        using Inference = PolicyValueModel::Inference;
    };

    struct Inference
    {
        typename Types::Real value;
        typename Types::VectorReal policy;
    };

    void get_inference(typename Types::Inference &inference);
};

template <class State>
// requires std::derived_from<State, AbstractState>;
class MonteCarloModel : public PolicyValueModel<State>
{
public:
    struct Types : PolicyValueModel<State>::Types
    {
    };

    void get_inference(
        State state,
        typename Types::Inference &inference)
    {
        while (!state.terminal) {
            // TODO
        }
    }
};