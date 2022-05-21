#include "rational.h"

typedef unsigned short int Action;

struct State;

class MatrixNode;

class ChanceNode {

    public:
    MatrixNode* parent = nullptr;
    ChanceNode* next = nullptr; // Next ChanceNode 'entry' in a matrix node's matrix
    MatrixNode* child = nullptr; // Linked list of MatrixNodes consisting of the ChanceNode's branches

    int i, j; // Coords of the chance node 

    ChanceNode (MatrixNode* parent, int i, int j) :
        parent(parent), i(i), j(j) {}

    MatrixNode* access (int transitionKey, Rational transitionProb);

    unsigned int visits = 0;
    float q0 = 0.f;
    float q1 = 0.f;

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

    ChanceNode* access (int i, int j);

    void expand (State* S);

    // Cumulative visits and score
    // Expected score = q/visits
    unsigned int visits = 0;
    float q0 = 0.f;
    float q1 = 0.f;

    unsigned int m, n;

    // It might actually be faster to always initialize arrays of size 9 (max actions size)
    // I think the memory loss won't be an issue since there will be no extra child nodes
    //  with our linked list implementation, and all our nodes variables would be close in memory
    // With pointers, I think all of the action arrays are just floating out in the heap, yes?
    Action* actions0;
    Action* actions1;
    float* gains0;
    float* gains1;
    int* visits0;
    int* visits1;
    
    // Inference value and policies. Yes, even for a Monte-Carlo search.
    // The Search functions are currently recursive, and do not return the rollout value itself.
    // Instead, they return a pointer to the last node reached, and the rollout value
    // is, in the case of mcts, the inferenced value 'v'.
    // Maybe worth mentioning there are values for each player, so we can have 'double-loss'
    // To avoid repitition in the search tree, and have two models inferencing in one tree.
    float v0;
    float v1;
    float* p0;
    float* p1;
};