template <typename T>
struct A {
    T a;
};

struct Empty {};

int main () {
    A<Empty> x;
    return 0;
}