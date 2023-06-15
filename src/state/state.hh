#pragma once

#include <types/types.hh>

#include <concepts>
#include <vector>

template <class _Types>
class AbstractState
{
public:
    struct Types : _Types
    {
        using TypeList = _Types;
    };
};

template <class _Types>
class PerfectInfoState : public AbstractState<_Types>
{
public:
    struct Types : AbstractState<_Types>::Types
    {
    };

    PerfectInfoState() {}

    PerfectInfoState(typename Types::PRNG &device) {}

    bool is_terminal{false};
    typename Types::VectorAction row_actions{};
    typename Types::VectorAction col_actions{};
    typename Types::Value payoff{};
    typename Types::Observation obs{};
    typename Types::Probability prob{};
    // TODO where is seed member?
    void get_actions();

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action);

    void reseed(typename Types::PRNG &device){};

    void apply_action_indices(
        ActionIndex row_idx,
        ActionIndex col_idx);
};

template <class _Types>
class ChanceState : public PerfectInfoState<_Types>
{
public:
    struct Types : PerfectInfoState<_Types>::Types
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
        typename Types::Seed seed);
};

template <class _Types>
class SolvedState : public ChanceState<_Types>
{
public:
    struct Types : ChanceState<_Types>::Types
    {
    };

    void get_payoff_matrix(
        typename Types::MatrixValue &matrix);

    void get_strategies(
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy);
};