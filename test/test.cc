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
        double a = x.uniform();
        double b = y.uniform();
        double c = z.uniform();
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
    MonteCarlo<9> model(device);

    double eta = .02;
    int playouts = 10000000;

    for (int pp = 2; pp < 3; ++pp) {
        std::cout << pp << std::endl;

        ToyState<9> toy(device, 'w', pp, 3);
        MatrixNode<9, Exp3Stats<9>> root;

        Exp3SearchSession<9, ToyState<9>> session(device, &root, toy, model, eta);
        session.search(playouts);
        auto answer = session.answer();
        answer.print();

        std::cout << "forecast0 (odds): " << exp(root.s.gains0[0] * eta) << " " << exp(root.s.gains0[1] * eta) << std::endl;
        std::cout << "forecast1 (odds): " << exp(root.s.gains1[0] * eta) << " " << exp(root.s.gains1[1] * eta) << std::endl;
        std::cout << root.count() << " nodes" << std::endl;

    }

    //while (true) {}

}