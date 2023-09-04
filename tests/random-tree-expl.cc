#include <pinyon.hh>

/*

Test that tree bandit algorithms are producing low exploitability when searching on a random tree

*/

int main()
{

    // prng device{0};

    // Rational<> threshold = Rational<>{1, 2};
    // RandomTreeGenerator<> generator{
    //     prng{0},
    //     {1, 2},
    //     {2, 3},
    //     {1, 2},
    //     {Rational<>{1, 4}},
    //     std::vector<size_t>(100, 0)};

    // const double expl_threshold = .25;

    // using Types = MonteCarloModel<RandomTree<>>;
    // Types::Model model(device);
    // W::ModelWrapper<Types> model_wrapper(model);

    // W::SearchWrapper<TreeBandit<Exp3<Types>>> search_0{};
    // W::SearchWrapper<TreeBanditThreaded<Exp3<Types>>> search_1{};
    // W::SearchWrapper<TreeBanditThreadPool<Exp3<Types>>> search_2{};
    // search_1.ptr->threads = 4;
    // search_2.ptr->threads = 4;

    // std::vector<W::Search *> searchs = {&search_0, &search_1, &search_2};

    // const size_t search_iterations = 1000;

    // for (RandomTree<> &&state : generator)
    // {
    //     state.get_actions();

    //     TraversedState<Types> traversed_state(state, model);
    //     Matrix<PairReal<double>> payoff_matrix{traversed_state.current_node->stats.nash_payoff_matrix};

    //     W::StateWrapper<RandomTree<>> state_wrapper{state};

    //     for (W::Search *search_ptr : searchs)
    //     {
    //         std::vector<double> r{}, c{};
    //         search_ptr->run_and_get_strategies(r, c, search_iterations, state_wrapper, model_wrapper);
    //         double expl = math::exploitability(payoff_matrix, r, c);

    //         std::cout << "expl: " << expl << std::endl;
    //         if (expl > expl_threshold)
    //         {
    //             std::cout << "ERROR" << std::endl;
    //             exit(1);
    //         }
    //     }
    //     std::cout << std::endl;

    // }

    // return 0;
}