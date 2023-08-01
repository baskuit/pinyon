#pragma once

#include <types/types.hh>

#include <concepts>
#include <vector>

template <IsTypeList Types>
class AbstractState
{
};

template <typename Types>
concept IsStateTypes =
    requires(
        typename Types::State &state,
        typename Types::VectorAction &vec,
        typename Types::PRNG &device) {
        {
            state.is_terminal()
        } -> std::same_as<bool>;
        {
            state.get_payoff()
        } -> std::same_as<typename Types::Value>;
        {
            state.get_actions(
                vec, vec)
        } -> std::same_as<void>;
        {
            state.randomize_transition(device)
        } -> std::same_as<void>;
    } &&
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
        } -> std::same_as<typename State::Types::VectorAction &>;
        {
            state.col_actions
        } -> std::same_as<typename State::Types::VectorAction &>;
        {
            state.payoff
        } -> std::same_as<typename State::Types::Value &>;
        {
            state.obs
        } -> std::same_as<typename State::Types::Observation &>;
        {
            state.prob
        } -> std::same_as<typename State::Types::Probability &>;
        {
            state.apply_actions(action, action)
        } -> std::same_as<void>;
        {
            state.get_actions()
        } -> std::same_as<void>;
    } &&
    IsStateTypes<Types>;

template <IsPerfectInfoStateTypes T>
class PerfectInfoState : public AbstractState<Types>
{
public:
    bool terminal{false};
    T::VectorAction row_actions{};
    T::VectorAction col_actions{};
    T::Value payoff{};
    T::Observation obs{};
    T::Probability prob{};

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
        typename Types::Observation &obs,
        std::vector<typename Types::Observation> &chance_actions) {
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