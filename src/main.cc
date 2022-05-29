#include <iostream>

#include <libsurskit/math.hh>
#include "state/toy_state.hh"

int main () {

    int pp = 2;
    int length = 0;

    float total = 0;
    int n = 10000;

    ToyStateInfo info('u', pp, length, pp / (float) (pp + 1));
    ToyState state = ToyState(info);

    for (int i = 0; i < n; ++i) {

        ToyState state_ = state;
        state_.rollout();

        total += state_.info.payoff;
    }
    std::cout << total / n << std::endl;
} 