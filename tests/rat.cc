#include <surskit.hh>

int main () {

    using State = RandomTree<RatTypes>;
    using Model = MonteCarloModel<State>;
    prng device{3486946038};
    // 4153956833
    State state{device, 1, 2, 2, 1, 0};
    Model model{0};


    TraversedState<Model> solved{state, model};

    std::cout << device.get_seed() << std::endl;


    return 0;
}