#include "state/toy_states.hh"
#include "search/exp3.hh"
#include <iostream>

int main () {
    int pp = 1;
    int length = 1;
    ToyState* state = new ToyState('u', false, 2, pp, length);
    Exp3SearchSession session = Exp3SearchSession(state, .01);
    
    session.search(1000);

    session.denoise();

    //root->expand(state, model);
    //State state_ = State(*state);
    //session.search(root, state);
    //std::cout << root->expanded;
}

// change this morning: states not rolling out and showing payoff correctly. Likely has to do with giving duplicate name
// states still probably not working since pp = 5 does nothing