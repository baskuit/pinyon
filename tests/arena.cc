#include <surskit.hh>

int main()
{

    using State = Arena<RandomTree<>, MonteCarloModel<RandomTree<>>>;
    using Model = MonteCarloModel<State>;
    using Algorithm = TreeBanditThreaded<Exp3<Model>, MatrixNode, ChanceNode>;

    std::vector<W::SearchWrapper<TreeBandit<Exp3<MonteCarloModel<RandomTree<>>>, MatrixNode, ChanceNode>>> agents = {
        {.01}, {.1}, {.2}};

    prng device{0};
    W::ModelWrapper<MonteCarloModel<RandomTree<>>> model{device};

    RandomTreeGenerator generator{0, {1}, {4}, {1}, {0.0}, {0}};

    for (RandomTree<> &&random_tree : generator)
    {

        State arena{random_tree, model, agents};

        Model arena_model{1337};
        Algorithm session{};
        session.threads = 6;
        MatrixNode<Algorithm> root;

        session.run(12, device, arena, arena_model, root);
        State::Types::VectorReal row_strategy, col_strategy;
        session.get_empirical_strategies(root.stats, row_strategy, col_strategy);
        math::print(row_strategy);
        math::print(col_strategy);
    }

    return 0;
}