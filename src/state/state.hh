#pragma once

#include "../libsurskit/math.hh"

#include <concepts>

class AbstractState
{
public:
    struct Transition
    {
    };
    struct Actions
    {
    };
};

class PartiallyObservableStochastic : public AbstractState
{
};

template <
    typename _Action,
    typename _Observation,
    typename _Probability,
    typename _Real,
    typename _VectorAction,
    typename _VectorReal,
    typename _VectorInt>
// Template type correctness
// requires std::floating_point<_Real>

class State : public PartiallyObservableStochastic
{
public:
    struct Transition;
    struct Actions;
    struct Types
    {
        using Action = _Action;
        using Observation = _Observation;
        using Probability = _Probability;
        using Real = _Real;
        using VectorAction = _VectorAction;
        using VectorReal = _VectorReal;
        using VectorInt = _VectorInt;
        using Transition = State::Transition;
        using Actions = State::Actions;
    };

    bool is_terminal = false;
    bool opaque = false;
    bool blind = false;

    typename Types::Real row_payoff, col_payoff;

    struct Transition : PartiallyObservableStochastic::Transition
    {
        typename Types::Observation obs;
        typename Types::Probability prob;
    };
    Transition transition;

    struct Actions : PartiallyObservableStochastic::Actions
    {
        typename Types::VectorAction row_actions;
        typename Types::VectorAction col_actions;
        int rows;
        int cols;
    };
    Actions actions;

    /*
    See readme about generallity of States.
    */

    void get_actions();
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action);
};

/*
Derived class of AbstractState most likely to see use.
*/

template <int size, typename _Action, typename _Observation, typename _Probability>
class StateArray : public State<
                       _Action,
                       _Observation,
                       _Probability,
                       double,
                       std::array<double, size>,
                       std::array<int, size>,
                       std::array<_Action, size>>
{
};

/*
This represents states that accept input for the chance player.
*/

template <
    typename _Action,
    typename _Observation,
    typename _Probablity,
    typename _Real,
    typename _VectorReal,
    typename _VectorInt,
    typename _VectorAction>
class StateChance : public State<_Action, _Observation, _Probablity, _Real, _VectorReal, _VectorInt, _VectorAction>
{
public:
    struct Types : State<_Action, _Observation, _Probablity, _Real, _VectorReal, _VectorInt, _VectorAction>
    {
    };
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action);
};