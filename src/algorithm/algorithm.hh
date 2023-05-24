#pragma once

#include <model/model.hh>

template <class _Model>
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