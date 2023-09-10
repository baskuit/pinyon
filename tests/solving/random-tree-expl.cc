#include <pinyon.hh>

/*

Test that tree bandit algorithms are producing low exploitability when searching on a random tree

*/

const double expl_threshold = .60;

template <typename Types, typename SolvedState>
void test_st(
    const typename Types::State &state,
    const SolvedState &solved_state)
{
    typename Types::PRNG device{0};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    typename Types::Search search{typename Types::BanditAlgorithm{.1}};
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
void test_mt(
    const typename Types::State &state,
    const SolvedState &solved_state)
{
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
void test_op(
    const typename Types::State &state,
    const SolvedState &solved_state)
{
    std::vector<typename Types::State> states{state};
    typename Types::PRNG device{0};
    typename Types::Model model{0};
    std::vector<typename Types::MatrixNode> roots{1};
    typename Types::Search search{typename Types::BanditAlgorithm{.1}};
    const size_t iterations = 1 << 12;
    const size_t actor_per = 1 << 3;

    search.run_for_iterations(iterations, actor_per, device, states, model, roots);
    typename Types::VectorReal row_strategy, col_strategy;
    typename Types::MatrixValue payoff_matrix;
    solved_state.get_matrix(payoff_matrix);
    search.get_empirical_strategies(roots[0].stats, row_strategy, col_strategy);
    auto expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);
    std::cout << search << " : " << expl << std::endl;
    assert(expl < typename Types::Real{expl_threshold});
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_st(
    const std::tuple<SearchTypes...> search_type_tuple,
    const State &state,
    const SolvedState &solved_state)
{
    (test_st<SearchTypes>(state, solved_state), ...);
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_mt(
    const std::tuple<SearchTypes...> search_type_tuple,
    const State &state,
    const SolvedState &solved_state)
{
    (test_mt<SearchTypes>(state, solved_state), ...);
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_op(
    const std::tuple<SearchTypes...> search_type_tuple,
    const State &state,
    const SolvedState &solved_state)
{
    (test_op<SearchTypes>(state, solved_state), ...);
}

int main()
{
    using BaseTypes = MonteCarloModel<RandomTree<>>;

    auto bandit_type_pack =
        TypePack<
            Exp3<BaseTypes>>{};
    auto node_template_pack =
        NodeTemplatePack<
            DefaultNodes,
            LNodes,
            DebugNodes,
            FlatNodes>{};
    auto st_search_type_tuple = search_type_generator<TreeBandit>(bandit_type_pack, node_template_pack);
    auto mt_search_type_tuple = search_type_generator<TreeBanditThreaded, TreeBanditThreadPool>(bandit_type_pack, node_template_pack);
    auto op_search_type_tuple = search_type_generator<OffPolicy>(bandit_type_pack, node_template_pack);


    prng device{0};
    BaseTypes::Model model(device);

    RandomTreeGenerator<> generator{
        prng{},
        {1, 2, 3},
        {2, 3},
        {1, 2},
        {Rational<>{0, 1}, Rational<>{1, 2}},
        std::vector<size_t>(3, 0)};

    for (const auto &wrapped_state : generator)
    {
        const BaseTypes::State state = (wrapped_state.unwrap<BaseTypes>());
        std::cout << "state seed: " << state.device.get_seed() << std::endl;
        auto solved_state = TraversedState<BaseTypes>::State{state, model};
        std::cout << "state size: " << solved_state.node->stats.matrix_node_count << std::endl;
        test_expl_st(st_search_type_tuple, state, solved_state);
        test_expl_mt(mt_search_type_tuple, state, solved_state);
        test_expl_op(op_search_type_tuple, state, solved_state);
    }

    return 0;
}