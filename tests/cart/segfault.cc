#include <pinyon.hh>

int main()
{
    std::cout << "both double mutex" << std::endl;
    {
        std::cout << "new search old mutex" << std::endl;
        using BaseTypes = MonteCarloModel<MoldState<2>>;
        using Types = TreeBanditThreadPool<Exp3<BaseTypes>>;

        for (size_t threads = 1; threads <= 8; ++threads)
        {
            Types::Search search{Types::BanditAlgorithm{}, threads};
            Types::MatrixNode root{};

            BaseTypes::State state{size_t{50}};
            BaseTypes::Model model{0};

            const size_t iterations = 1 << 20;
            prng device{0};
            auto start = std::chrono::high_resolution_clock::now();
            search.run_for_iterations(iterations, device, state, model, root);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "threads: " << threads << " time(ms): " << duration.count() << " count: " << root.count_matrix_nodes() << std::endl;
        }
    }
    return 0;
}