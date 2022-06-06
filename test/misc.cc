#include <iostream>
#include <assert.h>

struct S {
    int* a = nullptr;
    S () {}
    S (int* a) : a{a} {}
};

int main () {
    S x;
    assert(x.a == nullptr);
    int glob[4] = {1, 1, 1, 0};
    S y(glob);
    assert(y.a != nullptr);
    std::cout << glob[0] << glob[1] << glob[2] << glob[3] << std::endl;
    std::cout <<  ' ' << y.a[0] <<  ' ' << y.a[1] <<  ' ' << y.a[2] <<  ' ' << y.a[3] << std::endl;
    
}