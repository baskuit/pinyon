#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"

// Currently a sandbox for Linear stuff

int main () {
    using Vector = Linear::Vector<double, 9>;
    using Matrix = Linear::Matrix2D<Rational, 9>;
    Vector x(5);
    Matrix M(2, 2);
    Matrix N = M*M;
    N.print();
    // Rational x(3, 4);
    // std::cout << x << std::endl;
    std::cout << Rational(1, 4) + Rational(0, 1) << std::endl;
    return 0;
}