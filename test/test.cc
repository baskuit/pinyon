#include <assert.h>
#include <iostream>

#include "libsurskit/math.hh"
#include "state/state.hh"
#include "tree/node.hh"
#include "model/model.hh"
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



int main () {

    prng device;
    MoldState<9> state(device, 2);

    PairActions<9> actions;
    state.actions(actions);

    int playouts = 1000000;
    for (int playout = 0; playout < playouts; ++ playout) {
        auto state_ = state;
        MatrixNode<9, Exp3Stats<9>> root;
        ChanceNode<9, Exp3Stats<9>>* c0 = root.access(0, 0);
        // this is a memory leak
        // is the destructor for MatrixNode defined?
    }
    while (true) {}
    //std::cout << ' '<< std::endl;

    return 0;
}