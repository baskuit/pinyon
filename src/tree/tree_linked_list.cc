#include "tree_linked_list.hh"
#include "../state/toy_states.hh"

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

ChanceNode* MatrixNode::access (int row_idx, int col_idx) {
        if (this->child == nullptr) {
            ChanceNode* child = new ChanceNode(this, row_idx, col_idx);
            this->child = child;
            return child;
        }
        ChanceNode* current = this->child; 
        ChanceNode* previous = this->child;
        while (current != nullptr) {

            previous = current;
            if (current->row_idx == row_idx && current->col_idx == col_idx) {    
                return current;
            }
            current = current->next;
        }
        ChanceNode* child = new ChanceNode(this, row_idx, col_idx);
        previous->next = child;
        return child;
}