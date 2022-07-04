#pragma once

#include "tree/node.hh"


// Very general wrapper for Exp3, Exp3p, Exp3++, MatrixUCB (https://arxiv.org/abs/2006.05145)
// Should be the top level of library besides maybe some sugar wrapper used in evaulating performance


template <typename Model>
class Algorithm {
public:

    using action_t = typename Model::action_t;
    using hash_t = typename Model::hash_t;
    using pair_actions_t = typename Model::pair_actions_t;
    using transition_data_t = typename Model::transition_data_t;
    using state_t = typename Model::state_t;
    using model_t = Model;
    using inference_t = typename Model::InferenceData;

    struct MatrixStats {};
    struct ChanceStats {
        virtual double value0 () {
            return .5;
        };
        virtual double value1 () {
            return .5;
        };
    };
};