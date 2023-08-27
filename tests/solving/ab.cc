#include <surskit.hh>

/*



*/

RandomTree<>::State convert(
    const RandomTree<RandomTreeRationalTypes>::State &state,
    Rational<> threshold)
{
    RandomTree<>::State float_state{
        prng{state.device.get_seed()},
        state.depth_bound,
        state.rows,
        state.cols,
        state.transitions,
        threshold};
    return float_state;
}

struct Solve
{

    using TypesRational = MonteCarloModel<RandomTree<RandomTreeRationalTypes>>;
    using TypesFloat = MonteCarloModel<RandomTree<RandomTreeFloatTypes>>;

    using State = TypesRational::State;
    using StateFloat = TypesFloat::State;
    using Model = TypesRational::Model;
    using ModelFloat = TypesFloat::Model;

    FullTraversal<TypesRational>::MatrixNode root_full{};
    std::pair<TypesRational::Real, TypesRational::Real> full_value;

    FullTraversal<TypesFloat>::MatrixNode root_full_f{};
    std::pair<TypesFloat::Real, TypesFloat::Real> full_f_value;

    AlphaBeta<TypesRational>::MatrixNode root_ab{};
    std::pair<TypesRational::Real, TypesRational::Real> ab_value;
    AlphaBeta<TypesFloat>::MatrixNode root_ab_f{};
    std::pair<TypesFloat::Real, TypesFloat::Real> ab_f_value;

    using time_t = decltype(std::chrono::high_resolution_clock::now());
    time_t start, end;

    size_t time_full, time_full_f, time_ab, time_ab_f;
    size_t count_full, count_full_f, count_ab, count_ab_f;

    Solve(const State &state, Rational<> threshold)
    {

        StateFloat state_f = convert(state, threshold);

        prng device{0};

        Model model{device.uniform_64()};
        ModelFloat model_f{device.uniform_64()};

        FullTraversal<TypesRational>::Search session_full{};
        FullTraversal<TypesFloat>::Search session_full_f{};
        AlphaBeta<TypesRational>::Search session_ab{Rational<>{0}, Rational<>{1}};
        AlphaBeta<TypesFloat>::Search session_ab_f{Rational<>{0}, Rational<>{1}};

        auto run_lambda = [](size_t *result, const auto *state, auto *model, const auto *search, auto *root, auto *value)
        {
            time_t start, end;
            auto seed = (*state).device.get_seed();
            start = std::chrono::high_resolution_clock::now();
            auto state_ = *state;
            prng device{0};
            auto data = search->run(device, state_, *model, *root);
            *value = data;
            end = std::chrono::high_resolution_clock::now();
            *result = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        };

        std::thread threads[4]
        {
            std::thread{run_lambda, &time_full, &state, &model, &session_full, &root_full, &full_value},
            std::thread{run_lambda, &time_full_f, &state_f, &model_f, &session_full_f, &root_full_f, &full_f_value},
            std::thread{run_lambda, &time_ab, &state, &model, &session_ab, &root_ab, &ab_value},
            std::thread{run_lambda, &time_ab_f, &state_f, &model_f, &session_ab_f, &root_ab_f, &ab_f_value}
        };
        size_t thread_count = 0;
        for (auto &thread : threads)
        {
            thread.join();
            ++thread_count;
        }

        double alpha{ab_value.first};
        double beta{ab_value.second};
        double alpha_f{ab_f_value.first};
        double beta_f{ab_f_value.second};
        double value{root_full.stats.payoff.get_row_value()};
        double value_f{root_full_f.stats.payoff.get_row_value()};

        root_full.stats.nash_payoff_matrix.print();
        root_full_f.stats.nash_payoff_matrix.print();
        root_ab.stats.data_matrix.print();
        root_ab_f.stats.data_matrix.print();
        std::cout << alpha << " <= " << value << " <= " << beta << std::endl;
        std::cout << alpha_f << " <= " << value_f << " <= " << beta_f << std::endl;
        std::cout << std::endl;

        double eps = (1 / (double)(1 << 5));

        assert(alpha <= value);
        assert(beta >= value);

        assert(alpha_f - value_f <= eps); // if alpha > value_f, then only by a little
        assert(value_f - beta_f <= eps);  // if value_f > beta, then only by a little
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
    Rational<> threshold{0};
    RandomTreeGenerator<RandomTreeRationalTypes> generator{
        prng{0},
        {3},
        {5},
        {1},
        {threshold},
        std::vector<size_t>(100, 0)};

    double total_ratio = 0;
    int tries = 0;


    size_t counter = 0;
    for (auto wrapped_state : generator)
    {
        auto state = wrapped_state.unwrap<RandomTree<RandomTreeRationalTypes>>();

    auto copy = state.chance_strategies;

        Solve solve{state, threshold};
        solve.count();

        if (solve.count_ab / (double)solve.count_full < 1/4) {
            std::cout << std::endl;
        }

        std::cout << "TIME| full: " << solve.time_full << " full_f: " << solve.time_full_f << " ab: " << solve.time_ab << " ab_f: " << solve.time_ab_f << std::endl;
        std::cout << "COUNT| full: " << solve.count_full << " full_f: " << solve.count_full_f << " ab: " << solve.count_ab << " ab_f: " << solve.count_ab_f << std::endl;

        ++counter;
    }

    return counter;
}