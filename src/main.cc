#include <iostream>

#include <libsurskit/math.hh>
#include "state/toy_state.hh"

void print (SolvedStateInfo& info) {
    std::cout << info.rows << ", " << info.cols << std::endl;
    std::cout << info.strategy0[0] << ", " << info.strategy1[0] << std::endl;
    std::cout << info.terminal << ", " << info.payoff << std::endl;
}

void prng_copy_test () {
    prng x;
    std::cout << x.uniform() << std::endl;
    prng y = x;
    std::cout << x.uniform() << std::endl;
    std::cout << y.uniform() << std::endl;
}

void prng_copy_test2 () {
    prng x;
    std::cout << x.uniform() << std::endl;
    prng y;
    y = x;
    std::cout << x.uniform() << std::endl;
    std::cout << y.uniform() << std::endl;
}

int main () {
    
    prng_copy_test2();

} 