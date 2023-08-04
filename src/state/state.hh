#pragma once

#include <types/types.hh>

#include <concepts>
#include <vector>

template <typename State, typename VectorAction, typename Value, typename PRNG>
concept IsState =
    requires(
        State &state,
        VectorAction &actions,
        PRNG &device) {
        {
            state.is_terminal()
        } -> std::same_as<bool>;
        {
            state.get_payoff()
        } -> std::same_as<Value>;
        {
            state.get_actions(actions, actions)
        } -> std::same_as<void>;
        {
            state.randomize_transition(device)
        } -> std::same_as<void>;
    };

template <typename Types>
concept IsStateTypes =
    IsState<
        typename Types::State,
        typename Types::VectorAction,
        typename Types::Value,
        typename Types::PRNG> &&
    IsTypeList<Types>;

template <typename Types>
concept IsPerfectInfoStateTypes =
    requires(
        typename Types::State &state,
        typename Types::Action &action) {
        {
            state.terminal
        } -> std::same_as<bool &>;
        {
            state.row_actions
        } -> std::same_as<typename Types::VectorAction &>;
        {
            state.col_actions
        } -> std::same_as<typename Types::VectorAction &>;
        {
            state.payoff
        } -> std::same_as<typename Types::Value &>;
        {
            state.obs
        } -> std::same_as<typename Types::Obs &>;
        {
            state.prob
        } -> std::same_as<typename Types::Prob &>;
        {
            state.apply_actions(action, action)
        } -> std::same_as<void>;
        {
            state.get_actions()
        } -> std::same_as<void>;
    } &&
    IsStateTypes<Types>;

template <IsTypeList T>
class PerfectInfoState
{
public:
    bool terminal{false};
    T::VectorAction row_actions{};
    T::VectorAction col_actions{};
    T::Value payoff{};
    T::Obs obs{};
    T::Prob prob{};

    inline T::Value get_payoff()
    {
        return payoff;
    }

    inline bool is_terminal()
    {
        return terminal;
    }
};

template <typename Types>
concept IsChanceStateTypes =
    requires(
        typename Types::State &state,
        typename Types::Action &action,
        typename Types::Obs &obs,
        std::vector<typename Types::Obs> &chance_actions) {
        {
            state.get_chance_actions(chance_actions, action, action)
        } -> std::same_as<void>;
        {
            state.apply_actions(obs, action, action)
        } -> std::same_as<void>;
    } &&
    IsPerfectInfoStateTypes<Types>;

template <IsTypeList Types>
class SolvedState : public PerfectInfoState<Types>
{
public:
    Types::VectorReal row_strategy, col_strategy;
};

template <typename Types>
concept IsSolvedStateTypes =
    requires(
        typename Types::State state,
        typename Types::VectorReal &strategy,
        typename Types::MatrixValue &matrix) {
        {
            state.get_strategies(strategy, strategy)
        } -> std::same_as<void>;
        {
            state.get_matrix(matrix)
        } -> std::same_as<void>;
    } &&
    IsChanceStateTypes<Types>;