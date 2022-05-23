#pragma once

#include "../libsurskit/rational.hh"
#include "../model/model.hh"

typedef int Action;

struct State;

class MatrixNode;

class ChanceNode {

    public:
    MatrixNode* parent = nullptr;
    ChanceNode* next = nullptr; // Next ChanceNode 'entry' in a matrix node's matrix
    MatrixNode* child = nullptr; // Linked list of MatrixNodes consisting of the ChanceNode's branches

    int row_idx, col_idx;

    ChanceNode (MatrixNode* parent, int row_idx, int col_idx) :
        parent(parent), row_idx(row_idx), col_idx(col_idx) {}

    MatrixNode* access (int transitionKey, Rational transitionProb);

    int visits = 0;
    float cumulative_score0 = 0.f;
    float cumulative_score1 = 0.f;

};

class MatrixNode {

public:
    ChanceNode* parent = nullptr;
    MatrixNode* next = nullptr; // Next MatrixNode 'outcome' in the chance node above this
    ChanceNode* child = nullptr; // Linked list of the matrix entries

    int transitionKey;
    Rational transitionProb;

    MatrixNode (ChanceNode* parent, int transitionKey, Rational transitionProb) :
        parent(parent), transitionKey(transitionKey), transitionProb(transitionProb) {}
    MatrixNode () {
        parent = nullptr;
        transitionKey = 0;
        transitionProb = Rational(1, 1);
    }

    bool terminal = false;
    bool expanded = false;

    ChanceNode* access (int row_idx, int col_idx);

    void expand (State* state, Model* model);

    int visits = 0;
    float cumulative_score0 = 0.f;
    float cumulative_score1 = 0.f;

    int rows, cols;

    Action* actions0;
    Action* actions1;
    float* gains0;
    float* gains1;
    int* visits0;
    int* visits1;
    
    // much batter names imo
    float  value_estimate0;
    float  value_estimate1;
    float* strategy_prior0;
    float* strategy_prior1;
};
