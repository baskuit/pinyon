#include "state.hh"
#include "../tree/node.hh"
#include "../search/matrix_ucb.hh"
#include "../model/monte_carlo.hh"


/*
Contains just enough info to pseudo-randomly expand to a recursively solved game tree. 
*/

template <int size_>
class SeedState : public State<int, int, size_> {
    
    

    const int depth_bound = 0;
    
    PairActions<int, size_> actions (PlayerAction action0, PlayerAction action1) {
        return TransitionData(0, Rational(1));
    }
    // Deterministic currently

    void get_actions (int action0, int action1, TransitionData<int, size>& transition_data) {
        transition_data
    }

};

template <int size_>
class TreeState : public State<int, int, size_> {

    MatrixNode<MatrixUCB<MonteCarlo<SeedState<size_>>>>* root;
    MatrixNode<MatrixUCB<MonteCarlo<SeedState<size_>>>>* current;


};