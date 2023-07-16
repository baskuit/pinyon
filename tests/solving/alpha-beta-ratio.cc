#include <surskit.hh>

/*

Computes the savings of serialized alpha beta

*/

template <DoubleOracleModelConcept Model>
struct Solve
{

    using State = typename Model::Types::State;

    MatrixNode<FullTraversal<Model>> root_full;
    MatrixNode<AlphaBeta<Model>> root_ab;
    std::pair<typename Model::Types::Real, typename Model::Types::Real> ab_value;

    Solve(State &state, Model &model)
    {

        prng device{0};
        FullTraversal<Model> session_full{};
        auto state_copy = state;
        session_full.run(state_copy, model, &root_full);
        AlphaBeta<Model> session_ab{Rational<>{0}, Rational<>{1}};
        session_ab.teacher = &root_full;
        state_copy = state;

        ab_value = session_ab.run(device, state_copy, model, root_ab);
    }
};

int main()
{

    RandomTreeGenerator<RatTypes> generator{
        prng{1},
        {1, 2, 3},
        {2, 3},
        {1, 2},
        {Rational<>{0}},
        std::vector<size_t>(100, 0)};

    double total_ratio = 0;
    int tries = 0;


    size_t counter = 0;
    for (auto wrapped_state : generator) {
        auto state = *wrapped_state.ptr;
        MonteCarloModel<RandomTree<RatTypes>> model{0};
        Solve<MonteCarloModel<RandomTree<RatTypes>>> solve{state, model};

        auto a = static_cast<mpq_class>(solve.ab_value.first).get_d();
        auto b = static_cast<mpq_class>(solve.ab_value.second).get_d();
        auto c = static_cast<mpq_class>(solve.root_full.stats.payoff.get_row_value()).get_d();
        
        assert(a <= c);
        assert(c <= b);
        // if (a != b) {
        //     std::cout << "values: (" << a << ", " << b << "), " << c << std::endl;
        // }
        ++counter;
    }

    return 0;
}