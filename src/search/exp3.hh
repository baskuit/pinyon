#pragma once

#include <math.h>
#include "session.hh"
#include "../model/monte_carlo.hh"

class Exp3SearchSession : public SearchSession {
public:
    float eta;
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

    void forecast(float* forecasts, float* gains, int k);

};