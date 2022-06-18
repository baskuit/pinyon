#include <iostream>
#include <array>
#include <thread>

// // class MonteCarlo : public Model { ...
// template <typename StateType>
// class Model {
//     typedef StateType state_type;
//     InferenceData<StateType> inference () {
//         InferenceData<StateType> x;
//         return x;
//     }
// };

// // class Exp3pStats : public Stats { ...
// template <typename StateType> // Passing the StateType (and not just size) is 'necessary' because Exp3p needs policies, which ToyStates have but States dont.
// class Stats {
// public:
//     typedef StateType state_type;
//     std::array<double, StateType::state_size> gains0;
// };

// template <typename StatsType>
// class Node {
// public:
//     typedef StatsType stats_type;
//     StatsType stats;
// };

// // 'Session' is basically a container for the algorithm. As such it should own its StatsType
// template <typename StatsType>
// class Session {
// public:
//     typedef StatsType stats_type;
//     void search (int playouts) {};
// };

/*
// Example of derivation for Exp3p algorithm

template <typename StateType>
struct Exp3pStats;

template <typename StateType>
class Exp3pSession : public Session<StateType> {
    StateType& state;
    Model<StateType>& model;
    Node<Exp3pStats<StateType>>* root;

    Exp3pSession (StateType state, Model<StateType> model, Node<Exp3pStats<StateType>> root);

    Node<Exp3pStats<StateType>>* search ();   
};
*/





#include "state/state.hh"
int main () {


    prng device;
    typedef ToyState<9> ToyState;
    ToyState toy(device, 'u', 2, 0);
    // InferenceData<ToyState> inference;
    // MonteCarlo<ToyState> model(device);

    // ToyState::pair_actions_type pair;
    
    // int playouts = 1000000;
    // double total = 0;
    // for (int playout = 0; playout < playouts; ++playout) {
    //     ToyState toy_ = toy;
    //     inference = model.inference(toy_, pair);
    //     total += inference.value_estimate0;
    // }
    // std::cout << total / playouts << std::endl;

    // typedef Exp3p<MonteCarlo<ToyState>> exp3p;

    // MatrixNode<exp3p> root;
    // std::cout << (root.child == nullptr) << std::endl;
    // root.access(0, 0);
    // std::cout << (root.child == nullptr) << std::endl;
}