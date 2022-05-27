#include "tree_linked_list.hh"
#include "../state/toy_states.hh"
#include <iostream>

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

ChanceNode* MatrixNode::access (Action action0, Action action1) {
        if (this->child == nullptr) {
            ChanceNode* child = new ChanceNode(this, action0, action1);
            this->child = child;
            return child;
        }
        ChanceNode* current = this->child; 
        ChanceNode* previous = this->child;
        while (current != nullptr) {

            previous = current;
            if (current->action0 == action0 && current->action1 == action1) {    
                return current;
            }
            current = current->next;
        }
        ChanceNode* child = new ChanceNode(this, action0, action1);
        previous->next = child;
        return child;
}

void MatrixNode::expand (State* state, Model* model) {

    this->expanded = true;
    this->rows = state->rows;
    this->cols = state->cols;
    this->terminal = state->terminal;

    this->value_estimate0 =         state->rollout();
    this->value_estimate1 = 1 - this->value_estimate0;

    this->gains0 = new float[2]{0.f};
    this->gains1 = new float[2]{0.f};
    this->visits0 = new int[2]{0};
    this->visits1 = new int[2]{0};
} 