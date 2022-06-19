#pragma once

#include "tree/node.hh"

template <typename Model>
class Algorithm {
public:

    typedef typename Model::state_t state_t;
    typedef typename Model::action_t action_t;
    typedef typename Model::pair_actions_t pair_actions_t;
    typedef typename Model::transition_data_t transition_data_t;
    typedef Model model_t;
    typedef typename Model::InferenceData inference_t;

    struct MatrixStats {};
    struct ChanceStats {};
};