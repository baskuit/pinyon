#pragma once

#include <model/model.hh>

template <typename Types>
concept IsBanditAlgorithmTypes = requires(
                                     typename Types::BanditAlgorithm &bandit,
                                     typename Types::PRNG &device,
                                     typename Types::State &state,
                                     typename Types::Model &model,
                                     typename Types::MatrixStats &matrix_stats,
                                     typename Types::ChanceStats &chance_stats,
                                     typename Types::Outcome &outcome,
                                     typename Types::VectorReal &strategy,
                                     typename Types::ModelOutput &inference,
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
    // {
    //     bandit.initialize_stats(0, state, model, matrix_stats)
    // } -> std::same_as<void>;
    // {
    //     bandit.expand(state, matrix_stats, inference)
    // } -> std::same_as<void>;
    // {
    //     bandit.select(device, matrix_stats, outcome)
    // } -> std::same_as<void>;
    // {
    //     bandit.update_matrix_stats(matrix_stats, outcome)
    // } -> std::same_as<void>;
    // {
    //     bandit.update_chance_stats(chance_stats, outcome)
    // } -> std::same_as<void>;
} && IsValueModelTypes<Types>;

template <typename Types>
concept IsMultithreadedBandit =
    requires(
        typename Types::BanditAlgorithm &bandit,
        typename Types::PRNG &device,
        typename Types::MatrixStats &matrix_stats,
        typename Types::ChanceStats &chance_stats,
        typename Types::Outcome &outcome,
        typename Types::Mutex &mtx) {
        {
            bandit.select(device, matrix_stats, outcome, mtx)
        } -> std::same_as<void>;
        {
            bandit.update_matrix_stats(matrix_stats, outcome, mtx)
        } -> std::same_as<void>;
        {
            bandit.update_chance_stats(chance_stats, outcome, mtx)
        } -> std::same_as<void>;
    } &&
    IsBanditAlgorithmTypes<Types>;

template <typename Types>
concept IsOffPolicyBandit =
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
        typename Types::TreeAlgorithm &session,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        typename Types::MatrixNode &matrix_node) {
        typename Types::TreeAlgorithm{}; // default constructor is valid
        {
            session.run(0, device, state, model, matrix_node)
        } -> std::same_as<size_t>;
        {
            session.run_for_iterations(0, device, state, model, matrix_node)
        } -> std::same_as<size_t>;
    } &&
    IsBanditAlgorithmTypes<Types>;