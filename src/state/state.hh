#pragma once

#include <types/types.hh>

#include <concepts>
#include <vector>

template <IsTypeList _Types>
class AbstractState
{
public:
    struct Types : _Types
    {
        using TypeList = _Types;
    };
};

template <typename State>
concept IsState = requires(
                      State obj,
                      typename State::Types::VectorAction &vec,
                      typename State::Types::PRNG &device,
                      typename State::Types::Observation &obs,
                      typename State::Types::Probability &prob) {
    {
        obj.is_terminal()
    } -> std::same_as<bool>;
    {
        obj.get_payoff()
    } -> std::same_as<typename State::Types::Value>;
    {
        obj.get_actions(
            vec, vec)
    } -> std::same_as<void>;
    {
        obj.randomize_transition(device)
    } -> std::same_as<void>;
} && IsTypeList<typename State::Types>;
// This last part means that `typename State::Types::Real real; real.c` will autocomplete :)

template <IsTypeList _Types>
class PerfectInfoState : public AbstractState<_Types>
{
public:
    struct Types : AbstractState<_Types>::Types
    {
    };

    PerfectInfoState() {}

    PerfectInfoState(Types::PRNG &device) {}

    bool terminal{false};
    Types::VectorAction row_actions{};
    Types::VectorAction col_actions{};
    Types::Value payoff{};
    Types::Observation obs{};
    Types::Probability prob{};

    inline Types::Value get_payoff()
    {
        return payoff;
    }

    inline bool is_terminal()
    {
        return terminal;
    }
};

template <typename State>
concept IsPerfectInfoState = requires(
                                 State obj,
                                 typename State::Types::VectorAction &vec,
                                 typename State::Types::PRNG &device) {
    {
        obj.terminal
    } -> std::same_as<bool &>;
    {
        obj.row_actions
    } -> std::same_as<typename State::Types::VectorAction &>;
    {
        obj.col_actions
    } -> std::same_as<typename State::Types::VectorAction &>;
    {
        obj.payoff
    } -> std::same_as<typename State::Types::Value &>;
    {
        obj.obs
    } -> std::same_as<typename State::Types::Observation &>;
    {
        obj.prob
    } -> std::same_as<typename State::Types::Probability &>;
    {
        obj.apply_actions(
            typename State::Types::Action{},
            typename State::Types::Action{})
    } -> std::same_as<void>;
    {
        obj.get_actions()
    } -> std::same_as<void>;
} && IsState<State>;

template <typename _Types>
class ChanceState : public PerfectInfoState<_Types>
{
public:
    struct Types : PerfectInfoState<_Types>::Types
    {
    };
};

template <typename State>
concept IsChanceState = requires(
                            State obj,
                            std::vector<typename State::Types::Observation> &chance_actions) {
    requires IsPerfectInfoState<State>;
    {
        obj.get_chance_actions(
            chance_actions,
            typename State::Types::Action{},
            typename State::Types::Action{})
    } -> std::same_as<void>;
    {
        obj.apply_actions(
            typename State::Types::Observation{},
            typename State::Types::Action{},
            typename State::Types::Action{})
    } -> std::same_as<void>;
} && IsPerfectInfoState<State>;

template <typename _Types>
class SolvedState : public ChanceState<_Types>
{
public:
    struct Types : ChanceState<_Types>::Types
    {
    };

    Types::VectorReal row_strategy, col_strategy;
};

template <typename State>
concept IsSolvedState = requires(
                            State obj,
                            typename State::Types::VectorReal &strategy,
                            typename State::Types::MatrixValue &matrix) {
    requires IsChanceState<State>;
    {
        obj.get_strategies(strategy, strategy)
    } -> std::same_as<void>;
    {
        obj.get_matrix(matrix)
    } -> std::same_as<void>;
} && IsChanceState<State>;
