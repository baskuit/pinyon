#pragma once

#include "types/types.hh"

template <struct _Types>
class AbstractState
{
};

template <class _Types>
class PerfectInfoState : public AbstractState<_Types>
{
public:
    struct Actions;
    struct Transition;
    struct Types : _Types
    {
        using Actions = PerfectInfoState::Actions;
        using Transition = PerfectInfoState::Transition;
    };
    struct Actions
    {
        typename Types::VectorAction row_actions;
        typename Types::VectorAction col_actions;
    };

    void get_actions();

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action);

    void random_seed(typename Types::PRNG &device){};

    typename Types::Real row_payoff, col_payoff;
    bool is_terminal = false;
    typename Types::Seed seed;
};