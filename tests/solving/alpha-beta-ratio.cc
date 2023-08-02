#include <surskit.hh>

/*

Computes the savings of serialized alpha beta

*/

template <IsDoubleOracleModelTypes Types>
struct Solve
{

    using State = typename Types::State;
    using Model = typename Types::Model;
    using Full = FullTraversal<Types>;
    using AB = AlphaBeta<Types>;
    using ABO = AlphaBetaOld<Types>;

    typename Full::MatrixNode root_full{};
    typename AB::MatrixNode root_ab{};
    typename ABO::MatrixNode root_ab_old{};
    std::pair<typename Types::Real, typename Types::Real> ab_value;

    Solve(typename Types::State &state, Model &model)
    {
        prng device{0};
        Full session_full{};
        auto state_copy = state;
        // session_full.run(state_copy, model, &root_full);

        AB session_ab{Rational<>{0}, Rational<>{1}};
        // session_ab.teacher = &root_full;
        state_copy = state;
        ab_value = session_ab.run(device, state_copy, model, root_ab);

        ABO session_ab_old{};
        session_ab_old.run(state, model, &root_ab_old);

    }
};

int main()
{

    RandomTreeGenerator<RandomTreeRationalTypes> generator{
        prng{0},
        {4},
        {3},
        {7},
        {Rational<>{1, 20}},
        std::vector<size_t>(100, 0)};

    double total_ratio = 0;
    int tries = 0;


    using Types = MonteCarloModel<RandomTree<RandomTreeRationalTypes>::T>::T;


    size_t counter = 0;
    for (auto wrapped_state : generator) {
        auto state = *wrapped_state.ptr;

        Types::Model model{0};
        Solve<Types> solve{state, model};
        // std::cout << state.device.get_seed() << std::endl;

        // auto a = static_cast<mpq_class>(solve.ab_value.first).get_d();
        // auto b = static_cast<mpq_class>(solve.ab_value.second).get_d();
        // auto c = static_cast<mpq_class>(solve.root_full.stats.payoff.get_row_value()).get_d();

        // RealType<mpq_class> g {static_cast<ArithmeticType<mpq_class>>(solve.root_ab_old.stats.row_value)};

        // assert(g.value.get_d() == solve.ab_value.first.value.get_d());
        
        // assert(a <= c);
        // assert(c <= b);
        // if (a != b) {
        //     std::cout << "values: (" << a << ", " << b << "), " << c << std::endl;
        // }

        // mpz_t gc1, gc2;
        // mpz_init(gc1);
        // mpz_init(gc2);

        // mpz_gcd(gc1, 
        // solve.ab_value.first.value.get_num_mpz_t(), 
        // solve.ab_value.first.value.get_den_mpz_t());
        // mpz_gcd(gc2, 
        // solve.root_full.stats.payoff.get_row_value().value.get_num_mpz_t(), 
        // solve.root_full.stats.payoff.get_row_value().value.get_den_mpz_t());

        size_t full_count = solve.root_full.count_matrix_nodes();
        size_t ab_count = solve.root_ab.count_matrix_nodes();
        size_t ab_old_count = solve.root_ab_old.count_matrix_nodes();
        // assert (ab_count <= ab_old_count);
        total_ratio += ab_count / (double) ab_old_count;

        // std::cout << "full: " << full_count << std::endl;
        // std::cout << "ab: " << ab_count << std::endl;
        // std::cout << "ab old: " << ab_old_count << std::endl;
        // std::cout << a << ' ' << b << ' ' << c << std::endl;
        // std::cout << std::endl;

        ++counter;
        double avg_ratio = total_ratio / counter;
        std::cout << "avg ratio: " << avg_ratio << std::endl;
    }

    
    

    return 0;
}