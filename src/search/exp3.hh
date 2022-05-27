#pragma once

#include <math.h>
#include "session.hh"
#include "../model/monte_carlo.hh"

class Exp3SearchSession : private SearchSession {
public:
    float eta;
    Exp3SearchSession ();
        Exp3SearchSession (State* state, float eta) {
        this->root = new MatrixNode();
        this->state = state;
        this->model = new MonteCarlo();

        if (!root->expanded) {
            root->expand(this->state, this->model);
        }

        int x[root->rows];
        this->visits0 = x;
        int y[root->cols];
        this->visits1 = x;
    };
    Exp3SearchSession (MatrixNode* root, State* state, Model* model, float eta) {
        this->root = root;
        this->state = state;
        this->model = model;
        this->eta = eta;
    };

    MatrixNode* search (MatrixNode* matrix_node_current, State* state);
    void search (int playouts);
    SearchSessionData answer ();

private:
    void forecast(float* forecasts, float* gains, int k);
};