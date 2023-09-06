#include <pinyon.hh>

/*

Test that tree bandit algorithms are producing low exploitability when searching on a random tree

*/

const double expl_threshold = .25;

template <typename Types, typename SolvedState>
void test_matrix_node_count_single_thread(
    const typename Types::State &state,
    const SolvedState &solved_state)
{
    typename Types::PRNG device{0};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    typename Types::Search search{};
    const size_t iterations = 1 << 10;

    search.run(iterations, device, state, model, root);
    typename Types::VectorReal row_strategy, col_strategy;
    typename Types::MatrixValue payoff_matrix;
    solved_state.get_matrix(payoff_matrix);
    search.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    auto expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);
    assert(expl < typename Types::Real{expl_threshold});
}

template <typename Types, typename SolvedState>
void test_matrix_node_count_multi_thread(
    const typename Types::State &state,
    const SolvedState &solved_state)
{
    typename Types::PRNG device{0};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    const size_t threads = 2;
    typename Types::Search search{typename Types::BanditAlgorithm{.1}, threads};
    const size_t iterations = 1 << 10;

    search.run(iterations, device, state, model, root);
    typename Types::VectorReal row_strategy, col_strategy;
    typename Types::MatrixValue payoff_matrix;
    solved_state.get_matrix(payoff_matrix);
    search.get_empirical_strategies(root.stats, row_strategy, col_strategy);
    auto expl = math::exploitability(payoff_matrix, row_strategy, col_strategy);
    assert(expl < typename Types::Real{expl_threshold});
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_st(
    const std::tuple<SearchTypes...> search_type_tuple,
    const State &state,
    const SolvedState &solved_state)
{
    (test_matrix_node_count_single_thread<SearchTypes>(state, solved_state), ...);
}

template <typename... SearchTypes, typename State, typename SolvedState>
void test_expl_mt(
    const std::tuple<SearchTypes...> search_type_tuple,
    const State &state,
    const SolvedState &solved_state)
{
    (test_matrix_node_count_multi_thread<SearchTypes>(state, solved_state), ...);
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

    prng device{0};
    BaseTypes::Model model(device);

    RandomTreeGenerator<> generator{
        prng{0},
        {1, 2},
        {2, 3},
        {1, 2},
        {Rational<>{1, 2}},
        std::vector<size_t>(100, 0)};

    for (const auto &wrapped_state : generator)
    {
        const BaseTypes::State state = (wrapped_state.unwrap<BaseTypes>());
        auto solved_state = TraversedState<BaseTypes>::State{state, model};
        test_expl_st(st_search_type_tuple, state, solved_state);
        test_expl_mt(mt_search_type_tuple, state, solved_state);
    }

    return 0;
}