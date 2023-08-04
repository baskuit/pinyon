#include <surskit.hh>

using OldTypes = MonteCarloModel<MoldState<2>>;
using Types = TraversedState<MonteCarloModel<MoldState<2>>>;

int main () {

    OldTypes::State state{1};
    OldTypes::Model model{};
    Types::State x{state, model};
}