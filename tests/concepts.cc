#include <surskit.hh>

struct Foo
{
    bool terminal;
};

template <IsState State>
void foo()
{
    State state{2};
    typename State::Types::PRNG device{};
}

template <IsTypeList Types>
void bar() {

}

int main()
{
    foo<MoldState<2>>();
}
