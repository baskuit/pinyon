#include <iostream>

#include <libsurskit/math.hh>
#include "state/toy_state.hh"

int main () {

    int pp = 3;
    int length = 0;

    float total = 0;
    int n = 100000;


    for (int i = 0; i < n; ++i) {

        ToyStateInfo info('u', pp, length, pp / (float) (pp + 1));
        ToyState state = ToyState(info);
        state.rollout();
        total += state.info.payoff;
        //std::cout << state.info.payoff << std::endl;
    }
    std::cout << total / n << std::endl;
} 