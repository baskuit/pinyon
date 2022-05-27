#include "state/toy_states.hh"
#include "search/exp3.hh"
#include <iostream>

int main () {
    std::cout << '$' << std::endl;
    int pp = 3;
    int length = 1;
    ToyState* state = new ToyState('u', false, -1, pp, length);
    Exp3SearchSession session = Exp3SearchSession(state, .01);
    session.eta = .001;
    std::cout << "&&&" << std::endl;
    session.search(1000);
    session.answer();
    
    MatrixNode* leaf = session.search(session.root, session.state);

    Node* x = leaf->parent->parent;
    
    x->test();

}

// change this morning: states not rolling out and showing payoff correctly. Likely has to do with giving duplicate name
// states still probably not working since pp = 5 does nothingclear
