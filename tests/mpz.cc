#include <surskit.hh>

#include <iostream>

big_int x{1};

int main () {

    long long int y{*x};

    std::cout << y << std::endl;


    return 0;
}