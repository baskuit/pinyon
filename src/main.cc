#include "state/state.hh"
// #include "state/test_states.hh"
// #include "model/model.hh"

#include <iostream>

template <int size>
using SimpleTypes = TypeList<int, int, double, double, std::array<int, size>, std::array<double, size>, std::array<int, size>>;

int main()
{
    using SimpleTypes = SimpleTypes<4>;
    using State = State<SimpleTypes>;
    
    return 0;
}