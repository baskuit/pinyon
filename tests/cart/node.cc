#include <pinyon.hh>

int main()
{

    using BaseTypes = MonteCarloModel<MoldState<2>>;

    using SessionTypes0 = TreeBandit<Exp3<BaseTypes>>;
    using SessionTypes1 = TreeBanditThreaded<Exp3<BaseTypes>>;
    using SessionTypes2 = TreeBanditThreadPool<Exp3<BaseTypes>>;

    std::tuple<
        SessionTypes0,
        SessionTypes1,
        SessionTypes2>
        search_type_tuple{{}, {}, {}};

    std::tuple<
        SessionTypes0::Search,
        SessionTypes1::Search,
        SessionTypes2::Search>
        search_tuple{{}, {{}, 2}, {{}, 1}};

    auto type_search_zipped = zip(search_type_tuple, search_tuple);

    size_t lambda_idx = 0;

    auto lambda = [&lambda_idx](auto type_search_pair)
    {
        using Types = decltype(type_search_pair.first);
        using MatrixNode = typename Types::MatrixNode;
        MatrixNode root{};

        BaseTypes::State state{50};
        BaseTypes::Model model{0};

        const size_t iterations = 1 << 18;
        prng device{0};
        auto &search = type_search_pair.second;

        search.run_for_iterations(iterations, device, state, model, root);

        size_t count = root.count_matrix_nodes();

        double ratio = count / (double)iterations;

        std::cout << ratio << ", diff =" << std::abs(static_cast<double>(count) - static_cast<double>(iterations)) << std::endl;
        ++lambda_idx;
    };

    for (int i = 0; i < 100; ++i)
    {
        // Apply the lambda to all elements of the tuple using std::apply
        std::apply([&lambda](auto &...elements)
                   { (lambda(elements), ...); },
                   type_search_zipped);
    }

    return 0;
}