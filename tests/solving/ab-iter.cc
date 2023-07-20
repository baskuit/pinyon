#include <surskit.hh>

/*

Computes the savings of serialized alpha beta

*/

template <DoubleOracleModelConcept Model>
struct Solve
{

    using State = typename Model::Types::State;

    MatrixNode<FullTraversal<Model>> root_full{};
    MatrixNode<AlphaBeta<Model>> root_ab{};
    MatrixNode<AlphaBeta<Model>> root_ab_{};
    MatrixNode<AlphaBetaOld<Model>> root_ab_old{};
    std::pair<typename Model::Types::Real, typename Model::Types::Real> ab_value;

    Solve(const State &state, Model &model)
    {
        const int depth = 4;
        prng device{0};

        // FullTraversal<Model> session_full{};
        // auto state_copy = state;
        // // session_full.run(state_copy, model, &root_full);

        AlphaBeta<Model> session_ab{Rational<>{0}, Rational<>{1}};
        for (int d = 1; d <= depth; ++d) {
            session_ab.max_depth = d;
            ab_value = session_ab.run(device, state, model, root_ab);
        }

        AlphaBeta<Model> session_ab_{Rational<>{0}, Rational<>{1}};
        session_ab_.max_depth = depth;
        session_ab_.run(device, state, model, root_ab_);

        // AlphaBetaOld<Model> session_ab_old{};
        // session_ab_old.run(state, model, &root_ab_old);

    }
};

int main()
{

    RandomTreeGenerator<RatTypes> generator{
        prng{0},
        {7},
        {3},
        {3},
        {Rational<>{1, 20}},
        std::vector<size_t>(10, 0)};

    double total_ratio = 0;
    size_t counter = 0;
    for (auto wrapped_state : generator) {
        auto state = *wrapped_state.ptr;

        MonteCarloModel<RandomTree<RatTypes>> model{0};
        Solve<MonteCarloModel<RandomTree<RatTypes>>> solve{state, model};

        size_t count1, count2;
        count1 = solve.root_ab.count_matrix_nodes();
        count2 = solve.root_ab_.count_matrix_nodes(); 

        std::cout << count1 << ' ' << count2 << std::endl;

        double ratio = count1 / (double) count2;
        total_ratio += ratio;

        ++counter;
        std::cout << total_ratio / counter << std::endl;
    }

    return 0;
}