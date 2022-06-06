#include <iostream>
#include <assert.h> 

#include <libsurskit/math.hh>

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

    prng_copy_test_2 ();
    return 0;
}