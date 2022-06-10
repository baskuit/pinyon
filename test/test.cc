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


    MoldState<9> mold(device, 5);

    MonteCarlo<9> model(device);

    Exp3SearchSession<9> session(device, .0001);

    int playouts = 1000000;

    for (int pp = 0; pp < 10; ++pp) {
        MatrixNode<9, Exp3Stats<9>> root;
        for (int playout = 0; playout < playouts; ++ playout) {
            auto mold_ = ToyState<9>(device, 'u', 3);
            MatrixNode<9 , Exp3Stats<9>>* leaf = session.search(&root, mold_, model);
            //std::cout << pp << ' ' << leaf->inference.value_estimate0 << std::endl;

            
        }
        std::cout << root.count() << std::endl;
        std::cout << root.mean_value0() << std::endl;
    }

    while (true) {}
    //std::cout << ' '<< std::endl;



}