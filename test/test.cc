#include <assert.h>
#include <iostream>

#include "model/monte_carlo.hh"


int main () {

    using State = ToyState<9>;

    prng device;

    MonteCarlo<State> model(device);

    double total = 0;
    int playouts = 1000000;

    for (int p = 0; p < playouts; ++p) {
        State toy(device, 'u', 2, 0);
        State::pair_actions_t pair;
        model.inference(toy, pair);
        total += model.inference_.value_estimate0;
    }

    std::cout << (total / playouts) << std::endl;

    return 0;
}
