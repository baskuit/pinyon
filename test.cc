#include <assert.h>
#include <iostream>

struct A {
    int* a;
    A () {}
    A (int* a) : a(a) {

    }
};

int main () {
    A X;
    std::cout << X.a << std::endl;
}