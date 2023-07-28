#include <surskit.hh>

/*

Computes the savings of serialized alpha beta

*/

RandomTree<> convert(const RandomTree<RandomTreeRationalTypes> &state, Rational<> threshold)
{
    RandomTree<> x{
        prng{state.device.get_seed()},
        state.depth_bound,
        state.rows,
        state.cols,
        state.transitions,
        threshold};
    x.seed = state.seed;
    return x;
}

struct Solve
{

    using State = RandomTree<RandomTreeRationalTypes>;
    using StateFloat = RandomTree<>;
    using Model = MonteCarloModel<State>;
    using ModelFloat = MonteCarloModel<StateFloat>;

    MatrixNode<FullTraversal<Model>> root_full{};
    MatrixNode<FullTraversal<ModelFloat>> root_full_f{};

    MatrixNode<AlphaBeta<Model>> root_ab{};
    std::pair<typename Model::Types::Real, typename Model::Types::Real> ab_value;
    MatrixNode<AlphaBeta<ModelFloat>> root_ab_f{};
    std::pair<typename ModelFloat::Types::Real, typename ModelFloat::Types::Real> ab_f_value;

    using time_t = decltype(std::chrono::high_resolution_clock::now());
    time_t start, end;

    size_t time_full, time_full_f, time_ab, time_ab_f;
    size_t count_full, count_full_f, count_ab, count_ab_f;

    Solve(State &state, Rational<> threshold)
    {

        StateFloat state_f = convert(state, threshold);

        prng device{};

        Model model{device.uniform_64()};
        ModelFloat model_f{device.uniform_64()};

        FullTraversal<Model> session_full{};
        FullTraversal<ModelFloat> session_full_f{};
        AlphaBeta<Model> session_ab{Rational<>{0}, Rational<>{1}};
        AlphaBeta<ModelFloat> session_ab_f{Rational<>{0}, Rational<>{1}};

        start = std::chrono::high_resolution_clock::now();
        session_full.run(state, model, &root_full);
        end = std::chrono::high_resolution_clock::now();
        time_full = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        session_full_f.run(state_f, model_f, &root_full_f);
        end = std::chrono::high_resolution_clock::now();
        time_full_f = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // std::cout << "full matrix:" << std::endl;
        // root_full.stats.nash_payoff_matrix.print();
        // std::cout << "solutions:" << std::endl;
        // math::print(root_full.stats.row_solution);
        // math::print(root_full.stats.col_solution);
        // std::cout << "full float matrix:" << std::endl;
        // root_full_f.stats.nash_payoff_matrix.print();
        // std::cout << "solutions:" << std::endl;
        // math::print(root_full_f.stats.row_solution);
        // math::print(root_full_f.stats.col_solution);
        // std::cout << "ab matrix:" << std::endl;
        // root_ab.stats.data_matrix.print();

        start = std::chrono::high_resolution_clock::now();
        ab_value = session_ab.run(device, state, model, root_ab);
        end = std::chrono::high_resolution_clock::now();
        time_ab = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        ab_f_value = session_ab_f.run(device, state_f, model_f, root_ab_f);
        end = std::chrono::high_resolution_clock::now();
        time_ab_f = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        double alpha{ab_value.first.value.get_d()};
        double beta{ab_value.second.value.get_d()};
        double alpha_f{ab_f_value.first.value};
        double beta_f{ab_f_value.second.value};
        double value{root_full.stats.payoff.get_row_value().value.get_d()};
        double value_f{root_full_f.stats.payoff.get_row_value()};

        // std::cout << "ab matrix:" << std::endl;
        // root_ab.stats.data_matrix.print();

        double eps = (1 / (double)(1 << 5));

        assert(alpha <= value);
        assert(beta >= value);
        assert(alpha <= beta);

        assert(alpha_f - value_f <= eps);
        assert(beta_f - value >= -eps);
        assert(alpha_f - beta_f <= eps);

        assert(abs(alpha - alpha_f) < eps);
        assert(abs(beta - beta_f) < eps);
        assert(abs(value - value_f) < eps);
    }

    void count()
    {
        count_full = root_full.count_matrix_nodes();
        count_full_f = root_full_f.count_matrix_nodes();
        count_ab = root_ab.count_matrix_nodes();
        count_ab_f = root_ab_f.count_matrix_nodes();
    }
};

int main()
{
    Rational<> threshold = Rational<>{1, 2};
    RandomTreeGenerator<RandomTreeRationalTypes> generator{
        prng{0},
        {1, 2, 3},
        {2, 4},
        {1, 2, 4},
        {threshold},
        std::vector<size_t>(100, 0)};

    double total_ratio = 0;
    int tries = 0;

    size_t counter = 0;
    for (auto wrapped_state : generator)
    {
        auto state = *wrapped_state.ptr;

        Solve solve{state, threshold};
        solve.count();

        std::cout << "full: " << solve.time_full << " full_f: " << solve.time_full_f << " ab: " << solve.time_ab << " ab_f: " << solve.time_ab_f << std::endl;
        std::cout << "full: " << solve.count_full << " full_f: " << solve.count_full_f << " ab: " << solve.count_ab << " ab_f: " << solve.count_ab_f << std::endl;

        ++counter;
    }

    return counter;
}