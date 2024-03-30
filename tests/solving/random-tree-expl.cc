#include <pinyon.hh>

/*

Test that tree bandit algorithms are producing low exploitability when searching on a random tree

*/

const double expl_threshold = .60;

template <typename Types, typename SolvedState>
void test_st(const typename Types::State &state, const SolvedState &solved_state) {
    typename Types::PRNG device{0};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    typename Types::Search search{typename Types::BanditAlgorithm{.01}};
    const size_t iterations = 1 << 12;

    search.run_for_iterations(iterations, device, state, model, root);
    typename Types::VectorReal row_strategy, col_strategy;
    typename Types::MatrixValue payoff_matrix;
    solved_state.get_matrix(payoff_matrix);
    search.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    const auto expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);
    search.get_refined_strategies(root.stats, row_strategy, col_strategy);
    const auto refined_expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);

    // std::cout << search << std::endl;
    // std::cout << "matrix:" << std::endl;
    // payoff_matrix.print();
    std::cout << "expl: " << expl << std::endl;
    std::cout << "refined expl: " << refined_expl << std::endl;

    // std::cout << "strategies:" << std::endl;
    // math::print(row_strategy);
    // math::print(col_strategy);
    // std::cout << "visits:" << std::endl;
    // math::print(root.stats.row_visits);
    // math::print(root.stats.col_visits);

    assert(expl < typename Types::Real{expl_threshold});
}

template <typename Types, typename SolvedState>
void test_mt(const typename Types::State &state, const SolvedState &solved_state) {
    typename Types::PRNG device{0};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    const size_t threads = 8;
    typename Types::Search search{typename Types::BanditAlgorithm{.1}, threads};
    const size_t iterations = 1 << 15;

    search.run_for_iterations(iterations, device, state, model, root);
    typename Types::VectorReal row_strategy, col_strategy;
    typename Types::MatrixValue payoff_matrix;
    solved_state.get_matrix(payoff_matrix);
    search.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    auto expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);
    std::cout << search << " : " << expl << std::endl;
    assert(expl < typename Types::Real{expl_threshold});
}

template <typename Types, typename SolvedState>
void test_mtp(const typename Types::State &state, const SolvedState &solved_state) {
    typename Types::PRNG device{0};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    const size_t threads = 8;
    const size_t pool_size = 64;
    typename Types::Search search{typename Types::BanditAlgorithm{.1}, threads, pool_size};
    const size_t iterations = 1 << 15;

    search.run_for_iterations(iterations, device, state, model, root);
    typename Types::VectorReal row_strategy, col_strategy;
    typename Types::MatrixValue payoff_matrix;
    solved_state.get_matrix(payoff_matrix);
    search.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    auto expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);
    std::cout << search << " : " << expl << std::endl;
    assert(expl < typename Types::Real{expl_threshold});
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_st(const std::tuple<SearchTypes...> search_type_tuple, const State &state,
                  const SolvedState &solved_state) {
    (test_st<SearchTypes>(state, solved_state), ...);
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_mt(const std::tuple<SearchTypes...> search_type_tuple, const State &state,
                  const SolvedState &solved_state) {
    (test_mt<SearchTypes>(state, solved_state), ...);
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_mtp(const std::tuple<SearchTypes...> search_type_tuple, const State &state,
                   const SolvedState &solved_state) {
    (test_mtp<SearchTypes>(state, solved_state), ...);
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_op(const std::tuple<SearchTypes...> search_type_tuple, const State &state,
                  const SolvedState &solved_state) {
    (test_op<SearchTypes>(state, solved_state), ...);
}

int main() {
    using BaseTypes = MonteCarloModel<RandomTree<>>;

    auto bandit_type_pack = TypePack<Exp3<BaseTypes>, MatrixUCB<BaseTypes>>{};
    // auto node_template_pack =
    //     NodeTemplatePack<
    //         DefaultNodes,
    //         LNodes,
    //         DebugNodes,
    //         FlatNodes>{};
    auto node_template_pack = NodeTemplatePack<DefaultNodes>{};
    auto st_search_type_tuple = search_type_generator<TreeBandit>(bandit_type_pack, node_template_pack);
    auto mt_search_type_tuple = search_type_generator<TreeBanditThreaded>(bandit_type_pack, node_template_pack);
    auto mtp_search_type_tuple = search_type_generator<TreeBanditThreadPool>(bandit_type_pack, node_template_pack);
    auto op_search_type_tuple = search_type_generator<OffPolicy>(bandit_type_pack, node_template_pack);

    prng device{0};
    BaseTypes::Model model(device);

    RandomTreeGenerator<> generator{prng{5809346740957}, {1, 2}, {2}, {1, 3}, {0}, std::vector<size_t>(100, 0)};

    for (const auto &wrapped_state : generator) {
        const BaseTypes::State state = (wrapped_state.unwrap<BaseTypes>());
        // std::cout << "state seed: " << state.device.get_seed() << std::endl;
        auto solved_state = TraversedState<BaseTypes>::State{state, model};
        std::cout << "state size: " << solved_state.node->stats.matrix_node_count << std::endl;

        // std::cout << "chance strategies:" << std::endl;
        // math::print(state.chance_strategies);

        // std::cout << "apply chance actions demo:" << std::endl;
        // for (int i{}; i < state.rows; ++i) {
        //     for (int j{}; j < state.cols; ++j) {
        //         for (int c{}; c < 3; ++c) {
        //             std::cout << "-" << i << ' ' << j << ' ';
        //             auto state_copy{state};
        //             state_copy.apply_actions(i, j, c);
        //             std::cout << state_copy.prob << ' ' << state_copy.get_payoff() << std::endl;
        //         }
        //     }
        // }

        // std::cout << "apply actions demo:" << std::endl;
        // for (int i{}; i < state.rows; ++i) {
        //     for (int j{}; j < state.cols; ++j) {
        //         std::cout << "+" << i << ' ' << j << ' ';
        //         auto state_copy{state};
        //         state_copy.apply_actions(i, j);
        //         std::cout << state_copy.get_payoff() << std::endl;
        //     }
        // }

        test_expl_st(st_search_type_tuple, state, solved_state);
        // test_expl_mt(mt_search_type_tuple, state, solved_state);
        // test_expl_mtp(mtp_search_type_tuple, state, solved_state);
        // test_expl_op(op_search_type_tuple, state, solved_state);
    }

    return 0;
}