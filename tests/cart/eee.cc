#include <pinyon.hh>

auto bandit_type_pack = TypePack<Exp3<MonteCarloModel<MoldState<>>>>{};
auto node_template_pack = NodeTemplatePack<LNodes>{};

auto search_type_tuple = search_type_generator<TreeBandit>(bandit_type_pack, node_template_pack);
auto mt_search_type_tuple = search_type_generator<TreeBanditThreaded>(bandit_type_pack, node_template_pack);

template <typename Types>
void bar () {
    typename Types::PRNG device{0};
    typename Types::State state{2, 10};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    typename Types::Search search{};
    const size_t iterations = 1 << 10;
    search.run_for_iterations(iterations, device, state, model, root);
}

template <typename Types>
requires IsMultithreadedBanditTypes<Types>
void bar () {
    typename Types::PRNG device{0};
    typename Types::State state{2, 10};
    typename Types::Model model{0};
    typename Types::MatrixNode root{};
    const size_t threads = 4;
    typename Types::Search search{threads};
    const size_t iterations = 1 << 10;
    search.run_for_iterations(iterations, device, state, model, root);
}

template <typename... SearchTypes>
void foo (std::tuple<SearchTypes...> search_type_tuple) {
    (bar<SearchTypes>(), ...);
}

int main()
{
    foo(search_type_tuple);
    foo(mt_search_type_tuple);
}