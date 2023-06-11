#include <vector>
#include <array>
template <template <typename T> class Vector>
struct Types {};

template <int size>
struct ArrayWrapper {
    template <typename U>
    struct Array : std::array<U, size> {};
};

int main () {


    Types<std::vector> x;
    Types<ArrayWrapper<4>::Array> y;

}