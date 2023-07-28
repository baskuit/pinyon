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

    inline bool is_terminal () const {
        return terminal;
    } 

    void get_actions(){};

    void get_actions(
        Types::VectorAction &row_actions,
        Types::VectorAction &col_actions) const;

    void apply_actions(
        Types::Action row_action,
        Types::Action col_action);

    void randomize_transition(Types::PRNG &device);
};

template <typename _Types>
class ChanceState : public PerfectInfoState<_Types>
{
public:
    struct Types : PerfectInfoState<_Types>::Types
    {
    };

    void get_chance_actions(
        std::vector<typename Types::Observation> &chance_actions,
        Types::Action row_action,
        Types::Action col_action);

    void apply_actions(
        Types::Action row_action,
        Types::Action col_action,
        Types::Observation chance_action){};
};

template <typename _Types>
class SolvedState : public ChanceState<_Types>
{
public:
    struct Types : ChanceState<_Types>::Types
    {
    };

    Types::VectorReal row_strategy, col_strategy;

    void get_payoff_matrix(
        Types::MatrixValue &matrix);

    void get_strategies(
        Types::VectorReal &row_strategy,
        Types::VectorReal &col_strategy);
};

/*

Concepts

*/

// template <typename State>
// concept IsState = requires(State obj) {
//     obj.terminal;
// };

template <typename State>
concept IsPerfectInfoState = requires(State obj, typename State::Types::VectorAction &vec, typename State::Types::PRNG &device) {
    requires IsTypeList<typename State::Types>;
    std::is_same_v<decltype(obj.terminal), bool>;
    std::is_same_v<decltype(obj.row_actions), typename State::Types::VectorAction>;
    std::is_same_v<decltype(obj.col_actions), typename State::Types::VectorAction>;
    std::is_same_v<decltype(obj.payoff), typename State::Types::Value>;
    std::is_same_v<decltype(obj.obs), typename State::Types::Observation>;
    std::is_same_v<decltype(obj.prob), typename State::Types::Probability>;
    {
        obj.apply_actions(
            typename State::Types::Action{},
            typename State::Types::Action{})
    } -> std::same_as<void>;
    {
        obj.get_actions()
    } -> std::same_as<void>;
    {
        obj.get_actions(
            vec, vec)
    } -> std::same_as<void>;
    {
        obj.randomize_transition(device)
    } -> std::same_as<void>;
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
};

template <typename State>
concept IsSolvedState = requires(
    State obj,
    typename State::Types::VectorReal &strategy,
    typename State::Types::MatrixValue &matrix)

{
    requires IsChanceState<State>;
    {
        obj.get_strategies(strategy, strategy)
    } -> std::same_as<void>;
    {
        obj.get_matrix(matrix)
    } -> std::same_as<void>;
};