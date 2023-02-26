#pragma once

#include "../state/state.hh"

template <typename _State>
class Model
{
public:
    using State = _State;
    using PlayerAction = typename _State::PlayerAction;
    using ChanceAction = typename _State::ChanceAction;
    using Number = typename _State::Number;
    using VectorDouble = typename _State::VectorDouble;
    using VectorInt = typename _State::VectorInt;
    using VectorAction = typename _State::VectorAction;
    using TransitionData = typename _State::TransitionData;
    using PairActions = typename _State::PairActions;

    struct InferenceData
    {
        double value0 = .5;
        double value1 = .5;
    };

    InferenceData inference_data;

    Model () {}

    virtual void inference(State &state) = 0;
};