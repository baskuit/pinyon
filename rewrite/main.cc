#include "types.hh"
#include "state.hh"

#include <iostream>

int main () {

    struct Empty {};
    // using Types = Types<Empty, Empty, Empty, Empty, Empty>;
    State<SimpleTypes> state;
    // Types::Action row_action{};
    // state.apply_actions(row_action, row_action);
    std::cout << sizeof(state) << std::endl;


    return 0;
}