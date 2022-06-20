#pragma once

#include "../state/state.hh"

template <typename State>
class Model {
public:

    // This pattern ensures each higher type (Model, Nodes, etc) and their derivations have access to all the lower types.
    using action_t = typename State::action_t;
    using hash_t = typename State::hash_t;
    using pair_actions_t = typename State::pair_actions_t;
    using transition_data_t = typename State::transition_data_t;
    using state_t = State;

    struct InferenceData {};

    InferenceData inference_; //Bad naming. Fix!!!

    virtual InferenceData& inference (State& state, pair_actions_t& pair) = 0;
};