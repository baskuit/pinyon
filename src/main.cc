#include "state/toy_states.hh"
#include "search/exp3.hh"

int main () {
    int pp = 2;
    int length = 1;
    State* state = new ToyState('c', false, 0, pp, 1);
    auto test = new Exp3SearchSession(state, .01);
    //+ninety overloads
    return 0;
}