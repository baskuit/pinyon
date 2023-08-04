#include <surskit.hh>

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
        tuple{{}, {}, {}};

    std::vector<int> threads {1, 2, 2};

    auto lambda = [](auto type_list)
    {
        using Types = decltype(type_list);
        using MatrixNode = typename Types::MatrixNode; // Access the nested A class
        MatrixNode root{};

        BaseTypes::State state{50};
        BaseTypes::Model model{0};

        const size_t iterations = 1 << 18;
        prng device{0};
        typename Types::Search session{};
        session.run_for_iterations(iterations, device, state, model, root);

        size_t count = root.count_matrix_nodes();

        double ratio = count / (double) iterations;

        std::cout << ratio << std::endl;
    };

    // Apply the lambda to all elements of the tuple using std::apply
    std::apply([&lambda](auto &...elements)
               { (lambda(elements), ...); },
               tuple);

    return 0;
}