#include <surskit.hh>

/*

Test that tree bandit algorithms are producing low exploitability when searching on a random tree

*/

int main()
{

    // prng device(285608215);
    // std::vector<size_t> depth_bounds{1, 2};
    // std::vector<size_t> actions{2, 3, 4};
    // std::vector<size_t> chance_actions{1};
    // std::vector<double> transition_thresholds{0.0};
    // std::vector<size_t> states_per{10, 0};
    // RandomTreeGenerator generator{device, depth_bounds, actions, chance_actions, transition_thresholds, states_per};

    // const double expl_threshold = 1;

    // using Model = MonteCarloModel<RandomTree<>>;
    // Model model(device);
    // W::ModelWrapper<Model> model_wrapper(model);

    // W::SearchWrapper<TreeBandit<Exp3<Model>>> session_0{};
    // W::SearchWrapper<TreeBanditThreaded<Exp3<Model>>> session_1{};
    // W::SearchWrapper<TreeBanditThreadPool<Exp3<Model>>> session_2{};
    // session_1.ptr->threads = 4;
    // session_2.ptr->threads = 4;

    // std::vector<W::Search *> sessions = {&session_0, &session_1, &session_2};

    // const size_t iterations = 1000;

    // for (RandomTree<> &&state : generator)
    // {
    //     state.get_actions();

    //     TraversedState<Model> traversed_state(state, model);
    //     Matrix<PairDouble> payoff_matrix{traversed_state.current_node->stats.nash_payoff_matrix};

    //     W::StateWrapper<RandomTree<>> state_wrapper{state};

    //     for (W::Search *session_ptr : sessions)
    //     {
    //         std::vector<double> r{}, c{};
    //         r.resize(state.row_actions.size());
    //         c.resize(state.col_actions.size());
    //         session_ptr->run_and_get_strategies(r, c, iterations, state_wrapper, model_wrapper);
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

    return 0;
}