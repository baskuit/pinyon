#include <concepts>

template <typename State>
concept IsState = requires(State obj) {
    {
        obj.terminal
    } -> std::same_as<bool&>;
};

template <typename State>
concept IsPerfectInfoState = IsState<State> &&
requires(State obj) {
    requires IsState<State>;
};

struct Foo
{
    bool terminal;
};

template <IsPerfectInfoState State>
void foo()
{
    State state;
    state.terminal;
    state.ter
}

template <IsState State>
void foo()
{
    State state;
    state.terminal;
    
}

int main()
{
    foo<Foo>();
}
