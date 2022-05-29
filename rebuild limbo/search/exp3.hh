#pragma once

#include <math.h>
#include "session.hh"
#include "../model/monte_carlo.hh"

class Exp3SearchSession : public SearchSession {
public:
    float eta;
    State* state;
    Exp3SearchSession ();
        Exp3SearchSession (State* state, float eta) :
        state(state), eta(eta) {
        this->root = new MatrixNode();
        this->state = state;
        this->model = new MonteCarlo();

        if (!root->expanded) {
            root->expand(this->state->copy(), this->model);
        }

        int x[root->rows];
        this->visits0 = x;
        int y[root->cols];
        this->visits1 = x;
    };
    Exp3SearchSession (MatrixNode* root, State* state, Model* model, float eta) {
        if (!root->expanded) {
            root->expand(this->state, this->model);
        }
        this->root = root;
        this->state = state;
        this->model = model;
        this->eta = eta;
    };

    

    MatrixNode* search (MatrixNode* matrix_node_current, State* state);
    void search (int playouts);
    SearchSessionData answer ();

    void forecast(float* forecasts, float* gains, int k);
};