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

    float total = 0;

    // Maybe 

    prng device;

    ToyState<9> toy(device);
    MoldState<9> mold(device, 2);

    MonteCarlo<9> model(device);

    Exp3SearchSession<9> session;

    int playouts = 1000000;
    for (int playout = 0; playout < playouts; ++ playout) {
        auto toy_ = toy;
        total += (model.inference(toy_)).value_estimate0;

    }
    std::cout << total / playouts << std::endl;
    total = 0;

    MatrixNode<9, Exp3Stats<9>> root;
    ChanceNode<9, Exp3Stats<9>>* c0 = root.access(0, 0);

    session.expand(root, toy, model);


    playouts = 1;
    for (int playout = 0; playout < playouts; ++ playout) {
        auto toy_ = toy;
        session.search(root, toy_, model);

        root.s.gains0[0] += 100;

        std::array<float, 9> forecast0;
        std::array<float, 9> forecast1;

        session.forecast(forecast0, forecast1, root);
        for (int i = 0; i < 9; ++i) {
            std::cout << forecast0[i] << ' ';
        }
        std::cout << std::endl;
        
    }
    return 0;
    while (true) {}
    //std::cout << ' '<< std::endl;


}