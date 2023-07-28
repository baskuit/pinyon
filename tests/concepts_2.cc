#include <concepts>

template <typename Types>
concept IsTypeList = requires(Types obj) {
    // typename Types::Real {};
    typename Types::Real;

    // {
    //     typename Types::Real {}
    // } -> std::same_as<double>;
};

struct TypeList
{
    using Real = double;
};

template <IsTypeList Types>
void foo()
{
}

template <IsTypeList Types>
struct State {
};

int main() {
    foo<TypeList>();
    // State<TypeList>::T::Real;


}