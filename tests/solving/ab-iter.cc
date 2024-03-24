#include <pinyon.hh>

#include <chrono>

/*
Takes model types (requires the model be 'state-invariant')
and asserts that iterative deepening produces the same alpha/beta
when solving on the same state and same depth

We would also like to assert that the search requires less nodes
but that is simply not going to be the case all the time

*/

using SolvedTree = TraversedState<NullModel<RandomTree<RandomTreeRationalTypes>>>;


// struct ABIterTestData {
//     size_t time_us;
//     size_t iter_time_us;
//     size_t count;
//     size_t iter_count;
// };

// template <IsValueModelTypes Types>
// struct IterTest {

//     using Flat = AlphaBetaForce<Types>;
//     using Iter = AlpbaBetaIter<Types>

//     ABIterTestData test (const Types::State &) const {
//         size_t time_us, iter_time_us, count, count_iter;
        
//         return {time_us, iter_time_us, count, count_iter};
//     }

// };

using Types = AlphaBetaForce<MonteCarloModel<RandomTree<RandomTreeRationalTypes>>>;
using IterTypes = AlphaBetaIter<MonteCarloModel<RandomTree<RandomTreeRationalTypes>>>;
using FullTypes = FullTraversal<MonteCarloModel<RandomTree<RandomTreeRationalTypes>>>;

template <typename Types>
size_t count_matrix_nodes(const typename Types::MatrixNode *matrix_node)
{
    size_t c = 1;
    for (const auto &data : matrix_node->chance_data_matrix)
    {
        for (const auto &pair : data.branches)
        {
            const typename Types::MatrixNode *child = pair.second.matrix_node.get();
            c += count_matrix_nodes<Types>(child);
        }
    }
    return c;
}

void solve_check(
    const Types::State &state)
{
    using time_t = decltype(std::chrono::high_resolution_clock::now());
    time_t start, end;
    size_t us;

    const size_t depth = state.depth_bound;

    Types::PRNG device{0};
    Types::Model model{0};

    Types::Search search{1 << 0};
    IterTypes::Search search_iter{1 << 0};
    FullTypes::Search search_full{};

    Types::MatrixNode node{};
    IterTypes::MatrixNode node_iter{};
    FullTypes::MatrixNode node_full{};

    search_full.run(1000, device, state, model, node_full);

    start = std::chrono::high_resolution_clock::now();
    search.run(depth, device, state, model, node);
    end = std::chrono::high_resolution_clock::now();
    size_t ab_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "AB; time: " << ab_us << std::endl;

    size_t iter_us = 0;
    for (size_t d = 1; d <= depth; ++d)
    {
        start = std::chrono::high_resolution_clock::now();
        search_iter.run(d, device, state, model, node_iter);
        end = std::chrono::high_resolution_clock::now();
        us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "AB ITER; depth: " << d << " time: " << us << std::endl;
        iter_us += us;
    }
    std::cout << "AB ITER; total time: " << iter_us << std::endl;

    const size_t count = count_matrix_nodes<Types>(&node);
    const size_t count_iter = count_matrix_nodes<IterTypes>(&node_iter);


    double time_ratio = iter_us / (double) ab_us;
    double count_ratio = count_iter / (double) count;

    assert(node.alpha == node_iter.alpha);
    assert(node.beta == node_iter.beta);

    std::cout << "count ratio: " << count_ratio << std::endl;
    std::cout << "time ratio: " << time_ratio << std::endl;

    // std::cout << "V : " << node_full.stats.payoff.get_row_value().get_d() << std::endl;
    // std::cout << "AB: " << node.alpha.get_d() << ' ' << node.beta.get_d() << std::endl;
    // std::cout << "IT: " << node_iter.alpha.get_d() << ' ' << node_iter.beta.get_d() << std::endl;
    std::cout << "C : " << count << " >? " << count_iter << std::endl;
    
    // std::cout << "FT Matrix:" << std::endl;
    // node_full.stats.nash_payoff_matrix.print();
    // std::cout << "AB Matrix:" << std::endl;
    // node.chance_data_matrix.print();
    // std::cout << "IT Matrix:" << std::endl;
    // node_iter.chance_data_matrix.print();
    
    std::cout << std::endl;
}

SolvedTree::State generate_solved_tree (
    prng &device,
    const size_t depth,
    const size_t actions,
    const size_t transitions) {
    NullModel<RandomTree<RandomTreeRationalTypes>>::Model model{};
    return {{device, depth, actions, actions, transitions}, model};
}

int main()
{

    

    Types::Q threshold{0};
    RandomTreeGenerator<RandomTreeRationalTypes> generator{
        prng{2},
        {3, 4},
        {3, 4, 5},
        {1},
        {threshold},
        std::vector<size_t>(10, 0)};

    double total_ratio = 0;
    int tries = 0;

    size_t counter = 0;
    for (auto wrapped_state : generator)
    {
        auto state = wrapped_state.unwrap<RandomTree<RandomTreeRationalTypes>>();

        solve_check(state);
    }

    return counter;
}