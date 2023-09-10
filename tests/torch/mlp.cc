#include <pinyon.hh>

struct MoldStateTensorTypes : MoldState<>
{

    class Model : public TwoLayerMLP
    {
    public:
        Model() : TwoLayerMLP{386}
        {
        }

        void add_to_batch_input(
            MoldState<>::State &&state,
            torch::Tensor &model_batch_input) const
        {
            model_batch_input = torch::cat({model_batch_input, torch::rand({1, 386})});
        }
    };
};

template <typename Types>
void thread_test(
    typename Types::Model *batch_model,
    const size_t num_batches,
    const long int subbatch_size)
{
    auto output = batch_model->get_random_output(subbatch_size);
    for (size_t i = 0; i < num_batches; ++i)
    {
        auto input = batch_model->get_random_input(subbatch_size);
        batch_model->inference(input, output);
    }
}

template <typename Types>
double stress_test(
    typename Types::Model &model,
    const long int batch_size,
    const long int subbatches,
    const size_t threads,
    const size_t num_batches)
{
    using NewTypes = LibtorchBatchModel<Types>;
    typename NewTypes::Model batch_model{model, batch_size, subbatches};
    const size_t subbatch_size = batch_size / subbatches;
    std::thread thread_pool[threads];
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < threads; ++i)
    {
        thread_pool[i] = std::thread(thread_test<NewTypes>, &batch_model, subbatch_size, num_batches);
    }
    for (auto &t : thread_pool)
    {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return 0;
}

int main()
{

    using Types = MoldStateTensorTypes;

    const size_t batch_size = 1 << 8;
    const size_t minibatches = 1 << 2;

    Types::Model model{};

    stress_test<Types>(model, 256, 8, 8, 1 << 3);

    return 0;
}