#pragma once

#include "../model/model.hh"

class Node {

    Node* parent;
    Node* child;
    Node* next;

    float cumulativeScore0 = 0.f;
    float cumulativeScore1 = 0.f;
    int visits = 0;

public:
    Node () :
    parent(nullptr), child(nullptr), next(nullptr) {};
    Node (Node* parent) :
    parent(parent), child(nullptr), next(nullptr) {};
    ~Node () {
        delete child;
        delete next;
    }

    void update (float u0, float u1) {
        cumulativeScore0 += u0;
        cumulativeScore1 += u1;
        visits += 1;
    }
};

class MatrixNode;

class ChanceNode : public Node {

    MatrixNode* parent;
    MatrixNode* child;
    ChanceNode* next;

    Action action0;
    Action action1;

public:
    ChanceNode (MatrixNode* parent, Action action0, Action action1) :
    Node(), parent(parent), action0(action0), action1(action1) {}
    
    MatrixNode* access (StateTransitionData data);
};

class SearchStats {};

class MatrixNode : public Node {

    SearchStats stats;

    Hash transitionKey;
    Rational transitionProb;

public:
    MatrixNode () :
    Node(), transitionKey(0) {}
    MatrixNode (StateTransitionData data) :
    Node(), transitionKey(data.transitionKey), transitionProb(data.transitionProb) {}

    ChanceNode* access (Action action0, Action action1);

};