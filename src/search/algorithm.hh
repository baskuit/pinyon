#pragma once

template <typename _Model>
class Algorithm
{
public:
    using State = _Model;
    using PlayerAction = typename _Model::PlayerAction;
    using ChanceAction = typename _Model::ChanceAction;
    using Number = typename _Model::Number;
    using VectorDouble = typename _Model::VectorDouble;
    using VectorInt = typename _Model::VectorInt;
    using VectorAction = typename _Model::VectorAction;
    using TransitionData = typename _Model::TransitionData;
    using PairActions = typename _Model::PairActions;
    using Model = _Model;
    using InferenceData = typename _Model::InferenceData; 

    struct MatrixStats
    {
    };
    struct ChanceStats
    {
    };

    struct SearchParameters
    {
    };

    virtual void run () = 0;
};