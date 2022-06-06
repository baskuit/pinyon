#pragma once

#include "../model/model.hh"

class ChanceNode;

class SearchStats {};

class MatrixNode {
public:
    ChanceNode* parent = nullptr;
    ChanceNode* child = nullptr;
    MatrixNode* prev = nullptr;
    MatrixNode* next = nullptr;

    Hash transitionKey;
    Rational transitionProb;

    SearchStats* stats = nullptr;

    bool terminal = false;
    bool expanded = false;
    int rows = 1;
    int cols = 1;
    Action* actions0;
    Action* actions1;

    float value_estimate0 = .5f;
    float value_estimate1 = .5f;

    MatrixNode () :
    transitionKey(0) {}
    MatrixNode (ChanceNode* parent, MatrixNode* prev, StateTransitionData data) :
    parent(parent), prev(prev), transitionKey(data.transitionKey), transitionProb(data.transitionProb) {}
    ~MatrixNode ();

    ChanceNode* access (int action0, int action1);
    void print (int n);
};

class ChanceNode {
public:
    MatrixNode* parent = nullptr;
    MatrixNode* child = nullptr;
    ChanceNode* prev = nullptr;
    ChanceNode* next = nullptr;

    Action action0;
    Action action1;

    ChanceNode (MatrixNode* parent, ChanceNode* prev, Action action0, Action action1) :
    parent(parent), prev(prev), action0(action0), action1(action1) {}
    ~ChanceNode ();

    MatrixNode* access (StateTransitionData data);
    void print (int n);
};