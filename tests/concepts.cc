#include <surskit.hh>

template <IsTypeList Types>
void foo () {
}

int main () {

    foo<RandomTreeRationalTypes>();

    MoldState<2> x{1};

}