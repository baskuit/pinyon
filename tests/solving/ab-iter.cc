#include <chrono>
#include <pinyon.hh>

/*
Takes model types (requires the model be 'state-invariant')
and asserts that iterative deepening produces the same alpha/beta
when solving on the same state and same depth

We would also like to assert that the search requires less nodes
but that is simply not going to be the case all the time

*/
using T = RandomTree<RandomTreeRationalTypes>;

struct BiasModel : T {
    struct ModelOutput {
        T::Value value;
    };

    class Model {
       public:
        const int n{};

        Model() {}

        Model(int n) : n{n} {}

        void inference(T::State &&state, ModelOutput &output) const {
            if (state.payoff_bias < n) {
                output.value = T::Value{mpq_class{0}};
                return;
            }
            if (state.payoff_bias > n) {
                output.value = T::Value{mpq_class{1}};
                return;
            }
            output.value = T::Value{mpq_class{1, 2}};
        }
    };
};

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

//     ABIterTestData test (const BiasModel::State &) const {
//         size_t time_us, iter_time_us, count, count_iter;

//         return {time_us, iter_time_us, count, count_iter};
//     }

// };

using AB = AlphaBeta<BiasModel>;
using ABIter = AlphaBetaIter<BiasModel>;

void solve_check(const BiasModel::State &state) {
    assert(state.transitions == 1);

    using time_t = decltype(std::chrono::high_resolution_clock::now());
    time_t start, end;
    size_t us;

    const size_t max_depth = state.depth_bound;

    BiasModel::PRNG device{0};
    BiasModel::Model model{0};

    AB::Search search{};
    ABIter::Search search_iter{1, 1, 0.0};

    AB::MatrixNode node{};
    ABIter::MatrixNode node_iter{};

    const auto pair = search.run(max_depth, device, state, model, node);
    const auto idk = search_iter.run(max_depth, device, state, model, node_iter);

    // std::cout << "AB: " << pair.first.get_str() << ' ' << pair.second.get_str() << std::endl;
    // std::cout << "AB Iter: " << node_iter.alpha.get_str() << ' ' << node_iter.beta.get_str() << std::endl;
    std::cout << "AB: " << node.count_matrix_nodes() << std::endl;
    std::cout << "AB Iter: " << node_iter.count_matrix_nodes() << std::endl;

    assert(pair.first == node_iter.alpha);
    assert(pair.second == node_iter.beta);
}

int main() {
    BiasModel::Q threshold{0};
    RandomTreeGenerator<RandomTreeRationalTypes> generator{prng{2}, {2},      {8, 9},
                                                           {1},     {threshold}, std::vector<size_t>(10, 0)};

    double total_ratio = 0;
    int tries = 0;

    size_t counter = 0;
    for (auto wrapped_state : generator) {
        const auto state = wrapped_state.unwrap<T>();
        solve_check(state);
    }

    return 0;
}