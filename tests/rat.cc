#include <surskit.hh>

int main () {

    using State = RandomTree<RatTypes>;
    using Model = MonteCarloModel<State>;
    State state{0, 1, 2, 2, 2, 0};
    Model model{0};

    TraversedState<Model> solved{state, model};


    return 0;
}