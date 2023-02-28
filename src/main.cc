#include "state/test_states.hh"
#include <iostream>

int main()
{

    PennyMatching game;

    game.apply_actions(0, 0);
    std::cout << game.row_payoff << ' ' << game.col_payoff << std::endl;

    return 0;
}
