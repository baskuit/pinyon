#include <pinyon.hh>

auto bandit_type_pack = TypePack<Exp3<MonteCarloModel<MoldState<>>>>{};
auto node_template_pack = NodeTemplatePack<DefaultNodes, LNodes, DebugNodes, FlatNodes>{};

auto search_type_tuple = search_type_generator<TreeBandit>(bandit_type_pack, node_template_pack);
auto mt_search_type_tuple = search_type_generator<TreeBanditThreaded, TreeBanditThreadPool>(bandit_type_pack, node_template_pack);

template <typename Types>
void test_matrix_node_count_single_thread () {
    typename Types::PRNG device{0};
    typename Types::State state{2, 10};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    typename Types::Search search{};
    const size_t iterations = 1 << 10;

    search.run(iterations, device, state, model, root);
    std::cout << "single thread iterations: " <<  root.count_matrix_nodes() << std::endl;
}

template <typename Types>
// requires IsMultithreadedBanditTypes<Types>
void test_matrix_node_count_multi_thread () {
    typename Types::PRNG device{0};
    typename Types::State state{2, 10};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    const size_t threads = 2;
    typename Types::Search search{typename Types::BanditAlgorithm{.1}, threads};
    const size_t iterations = 1 << 10;

    search.run(iterations, device, state, model, root);
    std::cout << "multithread iterations (" << threads << " threads): " << root.count_matrix_nodes() << std::endl;
}

template <typename... SearchTypes>
void foo (std::tuple<SearchTypes...> search_type_tuple) {
    (test_matrix_node_count_single_thread<SearchTypes>(), ...);
}

template <typename... SearchTypes>
void foo_mt (std::tuple<SearchTypes...> search_type_tuple) {
    (test_matrix_node_count_multi_thread<SearchTypes>(), ...);
}

int main()
{
    foo(search_type_tuple);
    foo_mt(mt_search_type_tuple);
}