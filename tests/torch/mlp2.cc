#include <pinyon.hh>

struct RandomTreeLibtorchModel : RandomTree<>
{
    struct ModelOutput
    {
        RandomTree<>::Value value;
        RandomTree<>::VectorReal row_policy, col_policy;
    };

    struct Mask
    {
    };

    class Model : public TwoLayerMLP
    {
    public:
        Model() : TwoLayerMLP{386}
        {
        }

        void add_to_batch_input(
            RandomTree<>::State &&state,
            torch::Tensor &model_batch_input) const
        {
            model_batch_input = torch::cat({model_batch_input, torch::rand({1, 386})});
        }

        void get_mask(
            Mask &mask,
            const Types::State &state) const
        {
        }

        void get_output(
            ModelOutput &model_output,
            ModelBatchOutput &model_batch_output,
            const long int index,
            Mask &mask)
        {
            model_output.value = make_draw<RandomTree<>>();
            model_output.row_policy.resize(3);
            model_output.col_policy.resize(3);
            model_output.row_policy[0] = RandomTree<>::Real{1.0};
            model_output.col_policy[0] = RandomTree<>::Real{1.0};
        }

        void inference(
            RandomTree<>::State &&state,
            ModelOutput &output) const
        {
        }
    };
};

RandomTree<>::State gen(uint64_t seed)
{
    return RandomTree<>::State{
        prng{seed},
        20,
        3, 3,
        1};
}

template <CONCEPT(IsSearchTypes, Types)>
void off_policy_thread(
    typename Types::Model *batch_model_ptr,
    std::vector<typename Types::MatrixNode> *trees_ptr,
    std::vector<typename Types::State> *states_ptr,
    const size_t per_tree,
    const size_t total_steps,
    const size_t iterations)
{
    typename Types::Model &batch_model = *batch_model_ptr;
    std::vector<typename Types::MatrixNode> &trees = *trees_ptr;
    std::vector<typename Types::State> &states = *states_ptr;

    typename Types::Search off_policy_search;

    typename Types::PRNG device{};

    for (size_t step = 0; step < total_steps; ++step)
    {
        off_policy_search.run_for_iterations(iterations, per_tree, device, states, batch_model, trees);

        const size_t pairs = trees.size();
        for (size_t i = 0; i < pairs; ++i)
        {
            typename Types::State &state = states[i];
            typename Types::MatrixNode &tree = trees[i];
            typename Types::VectorReal row_strategy, col_strategy;
            off_policy_search.get_empirical_strategies(
                tree.stats,
                row_strategy,
                col_strategy);
            const auto row_action = state.row_actions[device.sample_pdf(row_strategy)];
            const auto col_action = state.col_actions[device.sample_pdf(col_strategy)];
            state.apply_actions(row_action, col_action);
            if (state.is_terminal())
            {
                state = gen(device.get_seed());
            }
            tree = typename Types::MatrixNode{};
        }
    }
}

template <CONCEPT(IsSearchTypes, Types)>
void off_policy_run(
    typename Types::Model &batch_model,
    const size_t threads,
    const size_t n_trees,
    const size_t per_tree,
    const size_t batch_size)
{
    std::vector<typename Types::MatrixNode> trees_for_thread[threads]{};
    std::vector<typename Types::State> states_for_thread[threads]{};

    for (size_t i = 0; i < threads; ++i)
    {
        for (size_t j = 0; j < n_trees; ++j)
        {
            states_for_thread[i].emplace_back(gen(prng{}.get_seed()));
            trees_for_thread[i] = std::vector<typename Types::MatrixNode>{n_trees};
        }
    }

    std::thread thread_pool[threads];
    auto start = std::chrono::high_resolution_clock::now();

    const size_t total_steps = 1 << 2;
    const size_t iterations = 1 << 10;

    double total_samples = threads * total_steps * per_tree * n_trees * iterations;
    std::cout << total_samples << std::endl;

    for (size_t i = 0; i < threads; ++i)
    {
        thread_pool[i] = std::thread(
            off_policy_thread<Types>,
            &batch_model,
            std::next(trees_for_thread, i),
            std::next(states_for_thread, i),
            per_tree,
            total_steps, iterations);
    }
    for (auto &t : thread_pool)
    {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "inference speed: " << total_samples / duration.count() * 1000 << std::endl;
}

int main()
{

    using Types = OffPolicy<Exp3<LibtorchBatchModel<RandomTreeLibtorchModel>>>;

    const size_t batch_size = 512;
    const size_t subbatches = 8;
    const size_t threads = subbatches;
    const size_t n_trees = 16;
    const size_t per_tree = 4;

    Types::Model batch_model{batch_size, subbatches};
    batch_model.to(torch::kCUDA);

    off_policy_run<Types>(
        batch_model,
        threads,
        n_trees,
        per_tree,
        batch_size);
}