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

    void test () {
        std::cout << '!' << std::endl;
    }
};

struct MatrixNode;

struct ChanceNode : Node {


// Linked list of MatrixNodes consisting of the ChanceNode's branches

    // row_idx changed to action0
    // works better for pruning
    MatrixNode* parent;
    MatrixNode* child;
    ChanceNode* next;
    Action action0, action1;

    ChanceNode (MatrixNode* parent, Action action0, Action action1) :
        parent(parent), action0(action0), action1(action1) {};

    int visits = 0;
    float cumulative_score0 = 0.f;
    float cumulative_score1 = 0.f;

    MatrixNode* access (int transitionKey, Rational transitionProb);
    void test () {
        std::cout << '@' << std::endl;
    }
};

struct MatrixNode : Node{

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

    ChanceNode* access (Action action0, Action actions1);

    void expand (State* state, Model* model);

    int visits = 0;
    float cumulative_score0 = 0.f;
    float cumulative_score1 = 0.f;

    int rows, cols;

    Action* actions0;
    Action* actions1;
    bool* pruned0;
    bool* pruned1;
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
