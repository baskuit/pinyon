#pragma once

#include <vector>

#include "../tree/node.hh"
#include "../state/state.hh"
#include "../model/model.hh"

struct SearchSessionData {};

class SearchSession {
public:

    prng* device;

    MatrixNode* root = nullptr;
    State* state = nullptr;
    Model* model = nullptr;

    // these belong to the session, not the node.
    int playouts_ = 0;
    std::vector<int> visits0_;
    std::vector<int> visits1_;

    SearchSession() {};
    SearchSession (prng* device, MatrixNode* root, State* state, Model* model) :
    device(device), root(root), state(state), model(model) {
        if (!root->expanded) {
            expand(root, state, model);
        }
        visits0_.resize(root->rows, 0);
        visits1_.resize(root->cols, 0);
    };

    virtual void search (int playouts) = 0;
    virtual MatrixNode* search (MatrixNode* matrix_node_current, State* state) = 0;
    virtual void expand (MatrixNode* matrix_node, State* state, Model* model) {};

};