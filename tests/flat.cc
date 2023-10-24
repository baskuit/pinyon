#include <pinyon.hh>

// model types
template <typename Types>
bool check_flat_search_equivalence(
    const size_t iterations,
    const typename Types::PRNG &device,
    const typename Types::State &state,
    const typename Types::Model &model,
    const typename Types::BanditAlgorithm &bandit)
{
    typename Types::MatrixStats stats, flat_stats;
    {
        auto device_ = device;
        auto model_ = model;
        typename TreeBandit<Types>::MatrixNode node{};
        typename TreeBandit<Types>::Search search{bandit};
        search.run_for_iterations(iterations, device_, state, model_, node);
        stats = node.stats;
    };
    {
        auto device_ = device;
        auto model_ = model;
        typename TreeBanditFlat<Types>::Search search{bandit};
        search.run(iterations, device_, state, model);
        flat_stats = search.matrix_stats[0];
    };
    return stats == flat_stats;
}

int main () {

    return 0;
}
