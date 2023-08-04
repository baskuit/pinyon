#include <concepts>

template <typename T>
concept C = requires (T t) {
    {
        T::A
    } -> std::same_as<const bool&>;// doesnt work
    {
        T::A
    } -> std::convertible_to<bool>; // works
};

struct Foo {
    static const bool A{false};
};

template <C T>
struct Test {
};

int main () {
    Test<Foo> {};
}