#pragma once

#include "../tree/tree_linked_list.hh"
#include "../state/state.hh"
#include "../model/model.hh"

class SearchSession {
public:
    MatrixNode* root;
    State* state;
    Model* model;

    int playouts;
    int* visits0;
    int* visits1;
    float cumulative_score0;
    float cumulative_score1;

    float* nash_solution0;
    float* nash_solution1;

    SearchSession() {};
    SearchSession (MatrixNode* root, State* state, Model* model) :
    root(root), state(state), model(model) {};

    virtual void search (int playouts) {};
    virtual MatrixNode* search (MatrixNode* matrix_node_current, State* state){
        return nullptr;
    };

/* 
    SearchSession operator+ (SearchSession session) {
        if (root != session.root) {
            
        }
        for (int i = 0; i < root->rows; ++i) {

        }
        SearchSession session_concat = {root, state, model,
        this->playout + session.playout, }
    }
*/
};