#include <surskit.hh>

int main()
{

    using BaseTypes = MonteCarloModel<MoldState<2>>;
    using Types = TreeBanditThreadPool<Exp3<BaseTypes>>;

    Types::Search search{Types::BanditAlgorithm{}, 2};
    // Types::Search search{{}, 2};


    for (int i = 0; i < 100; ++i)
    {

        Types::MatrixNode root{};

        BaseTypes::State state{50};
        BaseTypes::Model model{0};

        const size_t iterations = 1 << 18;
        prng device{0};

        search.run_for_iterations(iterations, device, state, model, root);

    }

    return 0;
}