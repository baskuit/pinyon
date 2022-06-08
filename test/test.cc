#include <iostream>

#include "libsurskit/math.hh"
#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3.hh"

// Passing a prng device to a function by value does not 'work'
// in the sense that calls inside the function will not progress the device outside the functions scope
// The reason for this is obvious

void prng_copy_test () {
    prng x;
    int n = 1000;
    for (int i = 0; i < n; ++i) {
        x.uniform();
    }
    prng y = x; // hard copy
    prng z = x.copy();

    for (int i = 0; i < n; ++i) {
        z.uniform();
    }
    for (int i = 0; i < n; ++i) {
        float a = x.uniform();
        float b = y.uniform();
        float c = z.uniform();
        assert (a == b);
        assert (b == c);
    }
}

void call_prng (prng device) {
    device.uniform();
}
void call_prng_ptr (prng* device) {
    (*device).uniform();
}

void prng_copy_test_2 () {
    prng x;
    prng y = x.copy();

    call_prng_ptr(&x);
    std::cout << x.uniform() << std::endl;
    std::cout << y.uniform() << std::endl;

}

void session_math () {

    // int rows = 2;
    // int cols = 2;
    
    // Action actions0[2] = {0, 1};
    // Action actions1[2] = {0, 1};
    // PairActions pair(2, 2, actions0, actions1);

    // SearchSessionData x(pair);
    // x.playouts = 999.f;
    // x.visits0_[0] = 666.f;
    // x.visits0_[1] = 333.f;
    // x.visits1_[0] = 666.f;
    // x.visits1_[1] = 333.f;
    // x.strategy0_[0] = 2/3.f;
    // x.strategy0_[1] = 1/3.f;
    // x.strategy1_[0] = 2/3.f;
    // x.strategy1_[1] = 1/3.f;

    // SearchSessionData y(pair);
    // y.playouts = 99.f;
    // y.visits0_[0] = 33.f;
    // y.visits0_[1] = 66.f;
    // y.visits1_[0] = 66.f;
    // y.visits1_[1] = 33.f;
    // y.strategy0_[0] = 1/3.f;
    // y.strategy0_[1] = 2/3.f;
    // y.strategy1_[0] = 1/3.f;
    // y.strategy1_[1] = 2/3.f;

    // auto z = x + y;
    // std::cout << z.strategy0_[0] << ' ' << z.strategy0_[1] << std::endl;

    
}

int main () {

    session_math ();
    return 0;
}