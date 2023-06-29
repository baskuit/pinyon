#include <surskit.hh>

/*

Computes the savings of serialized alpha beta

*/

template <class Model>
struct Solve
{

    using State = typename Model::Types::State;

    MatrixNode<FullTraversal<Model>> root_full;
    MatrixNode<AlphaBeta<Model>> root_ab;

    Solve(State &state, Model &model)
    {
        FullTraversal<Model> session_full{};
        auto state_copy = state;
        session_full.run(state_copy, model, &root_full);
        // std::cout << "real solve done- ";
        AlphaBeta<Model> session_ab{0, 1};
        // std::cout << "ab done." << std::endl;
        state_copy = state;

        session_ab.run(state_copy, model, &root_ab);
    }

    void report()
    {
        std::cout << root_ab.stats.row_value << ' ' << root_full.stats.payoff.get_row_value() << std::endl;
        std::cout << root_ab.stats.matrix_node_count << ' ' << root_full.stats.matrix_node_count << std::endl;
        std::cout << root_ab.stats.matrix_node_count / (double)root_full.stats.matrix_node_count << std::endl;
        return;
    }
};

int main()
{
    size_t tries = 1;
    double total_ratio = 0;
    for (int i = 0; i < tries; ++i)
    {
        uint64_t seed = 14258145225070533415;//prng{}.uniform_64();
        RandomTree state{seed, 1, 3, 3, 2, 0};
        MonteCarloModel<RandomTree<>> model{0};
        Solve<MonteCarloModel<RandomTree<>>> solve{state, model};

        auto error = solve.root_ab.stats.row_value - solve.root_full.stats.payoff.get_row_value();
        if (error > .05)
        {
            std::cout << seed << std::endl;
        }

        total_ratio += solve.root_ab.stats.matrix_node_count / (double)solve.root_full.stats.matrix_node_count;
    }

    std::cout << total_ratio / tries << std::endl;
}