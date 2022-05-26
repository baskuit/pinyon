#pragma once

#include <math.h>
#include "session.hh"
#include "../model/monte_carlo.hh"

class Exp3SearchSession : public SearchSession {
public:
    float eta;
    State* state;
    Exp3SearchSession ();
        Exp3SearchSession (State* state, float eta) {
        this->root = new MatrixNode();
        this->state = state;
        this->model = new MonteCarlo();
        this->eta = eta;
    };
    Exp3SearchSession (MatrixNode* root, State* state, Model* model, float eta) {
        this->root = root;
        this->state = state;
        this->model = model;
        this->eta = eta;
    };

    MatrixNode* search (MatrixNode* matrix_node_current, State* state);
    void search (int playouts);
    void denoise ();
    void forecast(float* forecasts, float* gains, int k);
    State* copy (); // :^) idx how else
};