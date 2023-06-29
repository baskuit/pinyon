#include <surskit.hh>

int main () {

    using State = RandomTree<RatTypes>;
    using Model = MonteCarloModel<State>;
    prng device{};
    State state{3669561725, 1, 2, 2, 1, 0};
    Model model{0};


    TraversedState<Model> solved{state, model};

    solved.current_node->stats.nash_payoff_matrix.print();
    std::cout << device.get_seed() << std::endl;
    std::cout << "payoff: " << solved.current_node->stats.payoff << std::endl;


    return 0;
}