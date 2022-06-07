#include "node.hh"

// Return MatrixNode child from chance node, with given Transition Key
// If child is not found, it is created
MatrixNode* ChanceNode :: access (StateTransitionData data) {
        if (this->child == nullptr) {
            MatrixNode* child = new MatrixNode(this, nullptr, data);
            this->child = child;
            return child;
        }
        MatrixNode* current = this->child; 
        MatrixNode* previous = this->child; 
        while (current != nullptr) {
            previous = current;
            if (current->transitionKey == data.transitionKey) {
                return current;
            }
            current = current->next;
        }
        MatrixNode* child = new MatrixNode(this, previous, data);
        previous->next = child;
        return child;
}

ChanceNode* MatrixNode :: access (Action action0, Action action1) {
        if (this->child == nullptr) {
            ChanceNode* child = new ChanceNode(this, nullptr, action0, action1);
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
        ChanceNode* child = new ChanceNode(this, previous, action0, action1);
        previous->next = child;
        return child;
}

MatrixNode :: ~MatrixNode() {
    delete stats;

    while (child != nullptr) {
        ChanceNode* victim = child;
        child = child->next;
        delete victim;
    }
    if (prev != nullptr) {
        prev->next = next;
    } else {
        parent->child = next;
    }
}

ChanceNode :: ~ChanceNode () {
    while (child != nullptr) {
        MatrixNode* victim = child;
        child = child->next;
        delete victim;
    }
    if (prev != nullptr) {
        prev->next = next;
    } else {
        parent->child = next;
    }
};

// void MatrixNode :: print (int n = 0) {
//     if (this == nullptr) {
//         return;
//     }
//     std::cout << n << ": " << this << std::endl;
//     if (n == 2){
//         return;
//     }
//     if (this->child != nullptr) {
//         this->child->print(n+1);
//     }
// }

// void ChanceNode :: print (int n = 0) {
//     if (this == nullptr) {
//         return;
//     }
//     std::cout << n << ": " << this << std::endl;
//     if (this->child != nullptr) {
//         this->child->print(n+1);
//     }
// }