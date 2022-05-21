#include "tree.h"
#include <iostream>
#include "toy_states.cc"

// Return MatrixNode child from chance node, with given Transition Key
// If child is not found, it is created
MatrixNode* ChanceNode::access (int transitionKey, Rational transitionProb) {
        if (this->child == nullptr) {
            MatrixNode* child = new MatrixNode(this, transitionKey, transitionProb);
            this->child = child;
            return child;
        }
        MatrixNode* current = this->child; 
        MatrixNode* previous = this->child; 
        while (current != nullptr) {
            previous = current;
            if (current->transitionKey == transitionKey) {
                return current;
            }
            current = current->next;
        }
        MatrixNode* child = new MatrixNode(this, transitionKey, transitionProb);
        previous->next = child;
        return child;
}

// Return ChanceNode child from matrix node, with given location on matrix
// If child is not found, it is created
// *Leaving in debug prints as comments*
ChanceNode* MatrixNode::access (int i, int j) {
        //std::cout << "Access Call" << std::endl;
        //std::cout << "Looking for: " << i << ' ' << j << std::endl;
        if (this->child == nullptr) {
            //std::cout << "No Child Node: Created, Returned" << std::endl;
            ChanceNode* child = new ChanceNode(this, i, j);
            this->child = child;
            return child;
        }
        ChanceNode* current = this->child; 
        ChanceNode* previous = this->child;
        //int depth = 0;
        while (current != nullptr) {
            //std::cout << "Following Path, Depth: " << depth << std::endl;
            //std::cout << "Currently at: " << current->i << ' ' << current->j << std::endl;
            previous = current;
            if (current->i == i && current->j == j) {
                //std::cout << "Node Found, Returned" << std::endl;      
                return current;
            }
            current = current->next;
            //depth += 1;
        }
        //std::cout << "No Node Found, Created, Returned" << std::endl;
        ChanceNode* child = new ChanceNode(this, i, j);
        previous->next = child;
        return child;
}

void MatrixNode::expand (State* S) {

    // I'm assumnig I shouldn't use "this->expanded = true" etc if I don't have to. Prolly slower?
    expanded = true;
    m = S->m;
    n = S->n;
    terminal = S->terminal;

    S->rollout();
    v0 = S->payoff;
    v1 = 1 - S->payoff;

    gains0 = new float[m]{0.f};
    gains1 = new float[n]{0.f};
    visits0 = new int[m]{0};
    visits1 = new int[n]{0};
}