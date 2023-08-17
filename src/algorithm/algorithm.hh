#pragma once

#include <model/model.hh>

template <typename Types>
concept IsBanditAlgorithmTypes =
    requires(
        typename Types::BanditAlgorithm &bandit,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixStats &matrix_stats,
        typename Types::ChanceStats &chance_stats,
        typename Types::Outcome &outcome,
        typename Types::VectorReal &strategy,
        typename Types::ModelOutput &model_output,
        typename Types::Value &value,
        typename Types::Real &real) {
        {
            bandit.get_empirical_strategies(matrix_stats, strategy, strategy)
        } -> std::same_as<void>;
        {
            bandit.get_empirical_value(matrix_stats, value)
        } -> std::same_as<void>;
        {
            bandit.get_refined_strategies(matrix_stats, strategy, strategy)
        } -> std::same_as<void>;
        {
            bandit.get_refined_value(matrix_stats, value)
        } -> std::same_as<void>;
        {
            bandit.initialize_stats(0, state, model, matrix_stats)
        } -> std::same_as<void>;
        {
            bandit.expand(state, matrix_stats, model_output)
        } -> std::same_as<void>;
        {
            bandit.select(device, matrix_stats, outcome)
        } -> std::same_as<void>;
        {
            bandit.update_matrix_stats(matrix_stats, outcome)
        } -> std::same_as<void>;
        {
            bandit.update_chance_stats(chance_stats, outcome)
        } -> std::same_as<void>;
    } &&
    IsValueModelTypes<Types>;

template <typename Types>
concept IsMultithreadedBanditTypes =
    requires(
        typename Types::BanditAlgorithm &bandit,
        typename Types::PRNG &device,
        typename Types::MatrixStats &matrix_stats,
        typename Types::ChanceStats &chance_stats,
        typename Types::Outcome &outcome,
        typename Types::Mutex &mutex) {
        {
            bandit.select(device, matrix_stats, outcome, mutex)
        } -> std::same_as<void>;
        {
            bandit.update_matrix_stats(matrix_stats, outcome, mutex)
        } -> std::same_as<void>;
        {
            bandit.update_chance_stats(chance_stats, outcome, mutex)
        } -> std::same_as<void>;
    } &&
    IsBanditAlgorithmTypes<Types>;

template <typename Types>
concept IsOffPolicyBanditTypes =
    requires(
        typename Types::BanditAlgorithm &bandit,
        typename Types::PRNG &device,
        typename Types::MatrixStats &matrix_stats,
        typename Types::ChanceStats &chance_stats,
        typename Types::Outcome &outcome,
        typename Types::VectorReal &strategy,
        typename Types::Real &real) {
        {
            bandit.update_matrix_stats(matrix_stats, outcome, real)
        } -> std::same_as<void>;
        {
            bandit.update_chance_stats(chance_stats, outcome, real)
        } -> std::same_as<void>;
        {
            bandit.get_policy(matrix_stats, strategy, strategy)
        } -> std::same_as<void>;
    } &&
    IsBanditAlgorithmTypes<Types>;

template <typename Types>
concept IsTreeBanditTypes =
    requires(
        typename Types::Search &session,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixNode &matrix_node) {
        {
            typename Types::Search{}
        };
        {
            session = session
        } -> std::same_as<typename Types::Search &>;
        {
            session.run(0, device, state, model, matrix_node)
        } -> std::same_as<size_t>;
        {
            session.run_for_iterations(0, device, state, model, matrix_node)
        } -> std::same_as<size_t>;
    } &&
    IsBanditAlgorithmTypes<Types>;

template <typename Types>
concept IsSearchTypes =
    requires(
        typename Types::Search &session,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixNode &matrix_node) {
        {
            session.run(0, device, state, model, matrix_node)
        } -> std::same_as<size_t>;
    } &&
    IsValueModelTypes<Types>;

// Is solver types?
// template <typename Types>
// concept IsSearchTypes =
//     requires(
//         typename Types::Search &session,
//         typename Types::PRNG &device,
//         typename Types::State &state,
//         typename Types::Model &model,
//         typename Types::MatrixNode &matrix_node) {
//         {
//             session.run(0, device, state, model, matrix_node)
//         } -> std::same_as<size_t>;
//     } &&
//     IsValueModelTypes<Types>;
