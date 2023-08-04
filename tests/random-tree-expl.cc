#include <surskit.hh>

/*

Test that tree bandit algorithms are producing low exploitability when searching on a random tree

*/

int main()
{

    prng device{0};

    Rational<> threshold = Rational<>{1, 2};
    RandomTreeGenerator<> generator{
        prng{0},
        {1, 2},
        {2, 3},
        {1, 2},
        {Rational<>{1, 4}},
        std::vector<size_t>(100, 0)};

    const double expl_threshold = .25;

    using Model = MonteCarloModel<RandomTree<>>;
    Model model(device);
    W::ModelWrapper<Model> model_wrapper(model);

    W::SearchWrapper<TreeBandit<Exp3<Model>>> session_0{};
    W::SearchWrapper<TreeBanditThreaded<Exp3<Model>>> session_1{};
    W::SearchWrapper<TreeBanditThreadPool<Exp3<Model>>> session_2{};
    session_1.ptr->threads = 4;
    session_2.ptr->threads = 4;

    std::vector<W::Search *> sessions = {&session_0, &session_1, &session_2};

    const size_t search_iterations = 1000;

    for (RandomTree<> &&state : generator)
    {
        state.get_actions();

        TraversedState<Model> traversed_state(state, model);
        Matrix<PairReal<double>> payoff_matrix{traversed_state.current_node->stats.nash_payoff_matrix};

        W::StateWrapper<RandomTree<>> state_wrapper{state};

        for (W::Search *session_ptr : sessions)
        {
            std::vector<double> r{}, c{};
            session_ptr->run_and_get_strategies(r, c, search_iterations, state_wrapper, model_wrapper);
            double expl = math::exploitability(payoff_matrix, r, c);

            std::cout << "expl: " << expl << std::endl;
            if (expl > expl_threshold)
            {
                std::cout << "ERROR" << std::endl;
                exit(1);
            }
        }
        std::cout << std::endl;

    }

    return 0;
}