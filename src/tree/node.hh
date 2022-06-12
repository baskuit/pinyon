#pragma once

#include "../model/model.hh"

template <int size, typename stats>
class ChanceNode;

struct stats {};


// Matrix Node


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

    ChanceNode<size, stats>* access (Action action0, Action action1) {
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
    };

    void update (double u0, double u1) {
        ++visits;
        cumulative_value0 += u0;
        cumulative_value1 += u1;
    }

    double mean_value0 () {
        return cumulative_value0 / visits;
    }

    double mean_value1 () {
        return cumulative_value1 / visits;
    }

    void make_terminal () {

    }

    int count () {
        int c = 1;
        auto current = child;
        while (current != nullptr) {
            c += current->count();
            current = current->next;
        }
        return c;
    }




    int visits = 0;
    double cumulative_value0 = 0.f;
    double cumulative_value1 = 0.f;

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

    int visits = 0;
    double cumulative_value0 = 0.f;
    double cumulative_value1 = 0.f;

    ChanceNode<size, stats> (MatrixNode<size, stats>* parent, ChanceNode<size, stats>* prev, Action action0, Action action1) :
        parent(parent), prev(prev), action0(action0), action1(action1) {}
   ~ChanceNode<size, stats> ();

    MatrixNode<size, stats>* access (StateTransitionData data) {
        if (this->child == nullptr) {
            MatrixNode<size, stats>* child = new MatrixNode<size, stats>(this, nullptr, data);
            this->child = child;
            return child;
        }
        MatrixNode<size, stats>* current = this->child; 
        MatrixNode<size, stats>* previous = this->child; 
        while (current != nullptr) {
            previous = current;
            if (current->transition_data.transitionKey == data.transitionKey) {
                return current;
            }
            current = current->next;
        }
        MatrixNode<size, stats>* child = new MatrixNode<size, stats>(this, previous, data);
        previous->next = child;
        return child;
    };

    void update (double u0, double u1) {
        ++visits;
        cumulative_value0 += u0;
        cumulative_value1 += u1;
    }

    int count () {
        int c = 0;
        auto current = child;
        while (current != nullptr) {
            c += current->count();
            current = current->next;
        }
        return c;
    }

    

};



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