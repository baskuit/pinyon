#include <surskit.hh>

template <IsPerfectInfoState State>
void foo () {
    State state{2};
}

int main()
{
    foo<MoldState<2>>();
}