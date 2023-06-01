#pragma once

#include <types/types.hh>

#include <concepts>
#include <vector>

template <class _Types, class _Value>
class AbstractState
{
public:
    struct Types : _Types {
        using TypeList = _Types;
        using Value = _Value;
    };
};

template <class _Types, class Value>
class State : public AbstractState<_Types>
{
public:
    static constexpr typename _Types::Real MIN_PAYOFF{0}, MAX_PAYOFF{1};
    static constexpr typename _Types::Real PAYOFF_SUM{MIN_PAYOFF + MAX_PAYOFF};
    static constexpr bool IS_CONSTANT_SUM = false;

    struct Types : AbstractState<_Types>::Types
    {
        using Value = Value<typename Types::Real, IS_CONSTANT_SUM, PAYOFF_SUM>;
    };

    State() {}

    bool is_terminal{false};
    typename Types::VectorAction row_actions{};
    typename Types::VectorAction col_actions{};
    typename Types::Value payoff{};
    typename Types::Observation obs{};
    typename Types::Probability prob{};
    typename Types::Seed seed{};

    void get_actions();

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action);

    void reseed(typename Types::PRNG &device){};

    void apply_action_indices(
        ActionIndex row_idx,
        ActionIndex col_idx)
    {
        apply_actions(row_actions[row_idx], col_actions[col_idx]);
    };
};

template <class _Types>
class ChanceState : public State<_Types>
{
public:
    struct Types : State<_Types>::Types
    {
    };

    void get_chance_actions(
        std::vector<typename Types::Observation> &chance_actions,
        int row_action,
        typename Types::Action col_action);

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action);

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Seed seed)
    {
        this->seed = seed;
        apply_actions(row_action, col_action);
    }
};