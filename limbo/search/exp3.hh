
#pragma once

#include "../libsurskit/math.hh"
#include "session.hh"
#include "../model/monte_carlo.hh"

struct Exp3SearchStats : SearchStats {
public:
    float* gains0 = nullptr;
    float* gains1 = nullptr;

    Exp3SearchStats () {}
    Exp3SearchStats (float* gains0, float* gains1) :
    gains0(gains0), gains1(gains1) {}
    ~Exp3SearchStats () {
        delete [] gains0;
        delete [] gains1;
    }
};

class Exp3SearchSession : public SearchSession {
public:

    float eta = .01f;
    
    Exp3SearchSession ();
    Exp3SearchSession (State* state, float eta) : 
    SearchSession(new MatrixNode(), state, new MonteCarlo()), eta(eta) {};
    Exp3SearchSession (MatrixNode* root, State* state, Model* model, float eta) :
    SearchSession(root, state, model), eta(eta) {};

    MatrixNode* search (MatrixNode* matrix_node_current, State* state);
    void search (int playouts);
    SearchSessionData answer ();

    void forecast(float* forecasts, float* gains, int k);
    void expand (MatrixNode* matrix_node, State* state, Model* model);
};