#pragma once

#include "../libsurskit/rational.hh"
#include "../model/model.hh"

#include <iostream>

typedef int Action;

struct State;

struct Node {
    Node* parent;
    Node* child;
    Node* next;

    int visits = 0;
    float cumulative_score0 = 0.f;
    float cumulative_score1 = 0.f;

    Node () {}
    ~Node() {
        delete child;
        delete next;
    }
};

struct MatrixNode;

struct ChanceNode : Node {
    MatrixNode* parent;
    MatrixNode* child;
    ChanceNode* next;

    Action action0, action1;

    ChanceNode (MatrixNode* parent, Action action0, Action action1) :
        parent(parent), action0(action0), action1(action1) {};

    MatrixNode* access (int transitionKey, Rational transitionProb);
};

struct MatrixNode : Node {

    ChanceNode* parent;
    MatrixNode* next; // Next MatrixNode 'outcome' in the chance node above this
    ChanceNode* child; // Linked list of the matrix entries

    int transitionKey;
    Rational transitionProb;

    bool terminal = false;
    bool expanded = false;

    int rows, cols;
    Action* actions0;
    Action* actions1;

    float* gains0;
    float* gains1;
    int* visits0;
    int* visits1;

    float  value_estimate0;
    float  value_estimate1;
    float* strategy_prior0;
    float* strategy_prior1;

    MatrixNode (ChanceNode* parent, int transitionKey, Rational transitionProb) :
        parent(parent), transitionKey(transitionKey), transitionProb(transitionProb) {}
    MatrixNode () {
        parent = nullptr;
        transitionKey = 0;
        transitionProb = Rational(1, 1);
    }

    ChanceNode* access (Action action0, Action actions1);

    void expand (State* state, Model* model);
};
