#pragma once

#include <model/model.hh>

template <IsValueModel _Model>
class AbstractAlgorithm
{
public:
    struct Types : _Model::Types
    {
        using Model = _Model;
    };
};

template <typename Algorithm>
concept IsBanditAlgorithm = requires(
    Algorithm &bandit,
    typename Algorithm::Types::PRNG &device,
    typename Algorithm::Types::State &state,
    typename Algorithm::Types::Model &model,
    typename Algorithm::Types::MatrixStats &matrix_stats,
    typename Algorithm::Types::ChanceStats &chance_stats,
    typename Algorithm::Types::Outcome &outcome,
    typename Algorithm::Types::VectorReal &strategy,
    typename Algorithm::Types::ModelOutput &inference,
    typename Algorithm::Types::Value &value,
    typename Algorithm::Types::Real &real) {
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
} && IsValueModel<typename Algorithm::Types::Model>;

template <typename Algorithm>
concept IsMultithreadedBandit =
    requires(
        Algorithm &bandit,
        typename Algorithm::Types::PRNG &device,
        typename Algorithm::Types::MatrixStats &matrix_stats,
        typename Algorithm::Types::ChanceStats &chance_stats,
        typename Algorithm::Types::Outcome &outcome,
        typename Algorithm::Types::Mutex &mtx) {
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
    IsBanditAlgorithm<Algorithm>;

template <typename Algorithm>
concept IsOffPolicyBandit =
    requires(
        Algorithm &bandit,
        typename Algorithm::Types::PRNG &device,
        typename Algorithm::Types::MatrixStats &matrix_stats,
        typename Algorithm::Types::ChanceStats &chance_stats,
        typename Algorithm::Types::Outcome &outcome,
        typename Algorithm::Types::VectorReal &strategy,
        typename Algorithm::Types::Real &real) {
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
    IsBanditAlgorithm<Algorithm>;

template <typename Algorithm>
concept IsTreeBandit =
    requires(
        Algorithm &session,
        typename Algorithm::Types::PRNG &device,
        typename Algorithm::Types::State &state,
        typename Algorithm::Types::Model &model,
        typename Algorithm::Types::MatrixNode &matrix_node) {
        Algorithm{};
        {
            session.run(0, device, state, model, matrix_node)
        } -> std::same_as<size_t>;
        {
            session.run_for_iterations(0, device, state, model, matrix_node)
        } -> std::same_as<size_t>;
    } &&
    IsBanditAlgorithm<Algorithm>;