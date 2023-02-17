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

    struct InferenceData {
        double value0 = .5; // wrong to call defacto value an estimate.
        double value1 = .5; // also storage for solved value, makes sense as inference property
    };

    InferenceData last_inference;

    virtual InferenceData& inference (State& state, pair_actions_t& legal_actions) = 0;
    // Intended that the last_inference member be updated and then returned
    // *note* perhaps this should be return type void and the arg should have inference_data. Does argument covariance work? dont remember tbh     
};