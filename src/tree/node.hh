#pragma once

#include "../model/model.hh"

template <int size, typename SearchStats>
class ChanceNode;

class SearchStats {};

template <int size, typename SearchStats>
class MatrixNode {
public:
    ChanceNode<size, SearchStats>* parent = nullptr;
    ChanceNode<size, SearchStats>* child = nullptr;
    MatrixNode<size, SearchStats>* prev = nullptr;
    MatrixNode<size, SearchStats>* next = nullptr;

    StateTransitionData transition_data;

    bool terminal = false; // used for pruned subtrees too
    bool expanded = false;

    PairActions<size>* pair;
    InferenceData<size> inference;
    SearchStats stats;

    int visits = 0;
    float cumulative_value0 = 0.f;
    float cumulative_value1 = 0.f;

    // MatrixNode () :
    // transitionKey(0) {}
    // MatrixNode (ChanceNode* parent, MatrixNode* prev, StateTransitionData data) :
    // parent(parent), prev(prev), transitionKey(data.transitionKey), transitionProb(data.transitionProb) {}
    // ~MatrixNode ();

    ChanceNode<size, SearchStats>* access (Action action0, Action action1);
};

template <int size, typename SearchStats>
class ChanceNode {
public:

    MatrixNode<size, SearchStats>* parent = nullptr;
    MatrixNode<size, SearchStats>* child = nullptr;
    ChanceNode<size, SearchStats>* prev = nullptr;
    ChanceNode<size, SearchStats>* next = nullptr;

    Action action0;
    Action action1;

    int visits = 0;
    float cumulative_value0 = 0.f;
    float cumulative_value1 = 0.f;

    ChanceNode (MatrixNode<size, SearchStats>* parent) {}
    ~ChanceNode ();

    MatrixNode<size, SearchStats>* access (StateTransitionData data);
};

template <int size, typename SearchStats>
MatrixNode<size, SearchStats>* ChanceNode<size, SearchStats> :: access (StateTransitionData data) {
        if (this->child == nullptr) {
            MatrixNode<size, SearchStats>* child = new MatrixNode(this, nullptr, data);
            this->child = child;
            return child;
        }
        MatrixNode<size, SearchStats>* current = this->child; 
        MatrixNode<size, SearchStats>* previous = this->child; 
        while (current != nullptr) {
            previous = current;
            if (current->transitionKey == data.transitionKey) {
                return current;
            }
            current = current->next;
        }
        MatrixNode<size, SearchStats>* child = new MatrixNode(this, previous, data);
        previous->next = child;
        return child;
}

// ChanceNode* MatrixNode :: access (Action action0, Action action1) {
//         if (this->child == nullptr) {
//             ChanceNode* child = new ChanceNode(this, nullptr, action0, action1);
//             this->child = child;
//             return child;
//         }
//         ChanceNode* current = this->child; 
//         ChanceNode* previous = this->child;
//         while (current != nullptr) {

//             previous = current;
//             if (current->action0 == action0 && current->action1 == action1) {    
//                 return current;
//             }
//             current = current->next;
//         }
//         ChanceNode* child = new ChanceNode(this, previous, action0, action1);
//         previous->next = child;
//         return child;
// }

// MatrixNode :: ~MatrixNode() {
//     delete stats;

//     while (child != nullptr) {
//         ChanceNode* victim = child;
//         child = child->next;
//         delete victim;
//     }
//     if (prev != nullptr) {
//         prev->next = next;
//     } else {
//         parent->child = next;
//     }
// }

// ChanceNode :: ~ChanceNode () {
//     while (child != nullptr) {
//         MatrixNode* victim = child;
//         child = child->next;
//         delete victim;
//     }
//     if (prev != nullptr) {
//         prev->next = next;
//     } else {
//         parent->child = next;
//     }
// };

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