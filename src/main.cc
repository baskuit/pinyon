#include <iostream>

#include "libsurskit/math.hh"
#include "state/toy_state.hh"
// #include "tree/node.hh"


int main () {
    prng device;
    ToyState x = ToyState(&device, 'u', 3, 0, .5f);
    prng device_new;
    ToyState x_copy = ToyState(&device_new);
    x.copy(&x_copy);
    std::cout << x_copy.pp << std::endl;

    return 0;
}