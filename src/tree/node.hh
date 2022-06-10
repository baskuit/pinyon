#pragma once

#include "../model/model.hh"

template <int size, typename stats>
class ChanceNode;

class stats {};

template <int size, typename stats>
class MatrixNode {
public:

    ChanceNode<size, stats>* parent = nullptr;
    ChanceNode<size, stats>* child = nullptr;
    MatrixNode<size, stats>* prev = nullptr;
    MatrixNode<size, stats>* next = nullptr;

    StateTransitionData transition_data;

    bool terminal = false; // used for pruned subtrees too
    bool expanded = false;

    PairActions<size> pair;
    InferenceData<size> inference;
    stats s;

    MatrixNode () {}
    MatrixNode (ChanceNode<size, stats>* parent, MatrixNode<size, stats>* prev, StateTransitionData transition_data) :
    parent(parent), prev(prev), transition_data(transition_data) {}
   ~MatrixNode ();

    ChanceNode<size, stats>* access (Action action0, Action action1);

    void update (float u0, float u1) {
        ++visits;
        cumulative_value0 += u0;
        cumulative_value1 += u1;
    }

    float mean_value0 () {
        return cumulative_value0 / visits;
    }

    float mean_value1 () {
        return cumulative_value1 / visits;
    }

    void make_terminal () {

    }



private:
    int visits = 0;
    float cumulative_value0 = 0.f;
    float cumulative_value1 = 0.f;

};


// Chance Node


template <int size, typename stats>
class ChanceNode {
public:


    MatrixNode<size, stats>* parent = nullptr;
    MatrixNode<size, stats>* child = nullptr;
    ChanceNode<size, stats>* prev = nullptr;
    ChanceNode<size, stats>* next = nullptr;

    Action action0;
    Action action1;
    int poo[100];
    int visits = 0;
    float cumulative_value0 = 0.f;
    float cumulative_value1 = 0.f;

    ChanceNode<size, stats> (MatrixNode<size, stats>* parent, ChanceNode<size, stats>* prev, Action action0, Action action1) :
        parent(parent), prev(prev), action0(action0), action1(action1) {}
   ~ChanceNode<size, stats> ();

    MatrixNode<size, stats>* access (StateTransitionData data);
};

template <int size, typename stats>
MatrixNode<size, stats>* ChanceNode<size, stats> :: access (StateTransitionData data) {
        if (this->child == nullptr) {
            MatrixNode<size, stats>* child = new MatrixNode<size, stats>(this, nullptr, data);
            this->child = child;
            return child;
        }
        MatrixNode<size, stats>* current = this->child; 
        MatrixNode<size, stats>* previous = this->child; 
        while (current != nullptr) {
            previous = current;
            if (current->transitionKey == data.transitionKey) {
                return current;
            }
            current = current->next;
        }
        MatrixNode<size, stats>* child = new MatrixNode<size, stats>(this, previous, data);
        previous->next = child;
        return child;
}

template <int size, typename stats>
ChanceNode<size, stats>* MatrixNode<size, stats> :: access (Action action0, Action action1) {
        if (this->child == nullptr) {
            ChanceNode<size, stats>* child = new ChanceNode<size, stats>(this, nullptr, action0, action1);
            this->child = child;
            return child;
        }
        ChanceNode<size, stats>* current = this->child; 
        ChanceNode<size, stats>* previous = this->child;
        while (current != nullptr) {

            previous = current;
            if (current->action0 == action0 && current->action1 == action1) {    
                return current;
            }
            current = current->next;
        }
        ChanceNode<size, stats>* child = new ChanceNode<size, stats>(this, previous, action0, action1);
        previous->next = child;
        return child;
}

template <int size, typename stats>
MatrixNode<size, stats> :: ~MatrixNode<size, stats>() {

    while (child != nullptr) {
        ChanceNode<size, stats>* victim = child;
        child = child->next;
        delete victim;
    }
    if (prev != nullptr) {
        prev->next = next;
    } else if (parent != nullptr) {
        parent->child = next;
    }
}

template <int size, typename stats>
ChanceNode<size, stats> :: ~ChanceNode<size, stats> () {
    while (child != nullptr) {
        MatrixNode<size, stats>* victim = child;
        child = child->next;
        delete victim;
    }
    if (prev != nullptr) {
        prev->next = next;
    } else if (parent != nullptr) {
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