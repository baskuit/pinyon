#pragma once

#include <model/model.hh>

template <IsValueModel _Model>
class AbstractAlgorithm
{
    static_assert(std::derived_from<_Model, AbstractModel<typename _Model::Types::State>>);

public:
    struct MatrixStats
    {
    };
    struct ChanceStats
    {
    };
    struct Types : _Model::Types
    {
        using Model = _Model;
        using MatrixStats = AbstractAlgorithm::MatrixStats;
        using ChanceStats = AbstractAlgorithm::ChanceStats;
    };
};

template <typename BanditAlgorithm>
concept IsBanditAlgorithm = requires(
    BanditAlgorithm &bandit,
    typename BanditAlgorithm::Types::PRNG &device,
    typename BanditAlgorithm::Types::State &state,
    typename BanditAlgorithm::Types::Model &model,
    typename BanditAlgorithm::Types::MatrixStats &matrix_stats,
    typename BanditAlgorithm::Types::ChanceStats &chance_stats,
    typename BanditAlgorithm::Types::Outcome &outcome,
    typename BanditAlgorithm::Types::VectorReal &strategy,
    typename BanditAlgorithm::Types::Real &real) {
    // {
    //     get_empirical_strategies(matrix_stats, strategy, strategy)
    // } -> std::same_as<void>;
    // {
    //     get_empirical_value(matrix_stats, real, real)
    // } -> std::same_as<void>;
    // {
    //     get_refined_strategies(matrix_stats, strategy, strategy)
    // } -> std::same_as<void>;
    // {
    //     get_refined_value(matrix_stats, real, real)
    // } -> std::same_as<void>;
    // {
    //     initialize_stats(size_t{}, state, model, matrix_stats)
    // } -> std::same_as<void>;
    // {
    //     bandit.select(device, matrix_stats, outcome)
    // } -> std::same_as<void>;
    {
        bandit.get_empirical_strategies(matrix_stats, strategy, strategy)
    } -> std::same_as<void>;
    // true;
};