#include <surskit.hh>

int main()
{

    using Model = MonteCarloModel<MoldState<2>>;

    auto session0 = TreeBandit<Exp3<Model>>{};
    auto session1 = TreeBanditThreaded<Exp3<Model>>{};
    auto session2 = TreeBanditThreadPool<Exp3<Model>>{};

    std::tuple<
        TreeBandit<Exp3<Model>>,
        TreeBanditThreaded<Exp3<Model>>,
        TreeBanditThreadPool<Exp3<Model>>>
        tuple{session0, session1, session2};

    std::get<1>(tuple).threads = 8;
    std::get<2>(tuple).threads = 8;

    auto lambda = [](auto &session)
    {
        using MatrixNode = typename std::remove_reference<decltype(session)>::type::Types::MatrixNode; // Access the nested A class
        MatrixNode root{};

        MoldState<2> state{50};
        Model model{0};

        const size_t iterations = 1 << 18;
        prng device{0};
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