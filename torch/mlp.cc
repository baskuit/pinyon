#include <pinyon.hh>

struct MoldStateTensorTypes : MoldState<>
{
    struct ModelBatchInput : torch::Tensor
    {
        ModelBatchInput(const long int size)
        {
        }
        void copy_range(ModelBatchInput &x, const long int a, const long int b)
        {
        }
    };
    struct ModelBatchOutput : torch::Tensor
    {
        ModelBatchOutput(const long int size)
        {
        }

        void copy_range(ModelBatchOutput &x, const long int a, const long int b)
        {
        }
    };

    class Model : public TwoLayerMLP
    {
    public:
        Model() : TwoLayerMLP{386}
        {
        }

        void inference(
            ModelBatchInput &batch_input,
            ModelBatchOutput &batch_output)
        {
            static_cast<torch::Tensor &>(batch_output) = this->forward(batch_input);
        }
    };
};

template <typename Types>
void thread_test(
    typename Types::Model *batch_model,
    const size_t num_batches,
    const long int subbatch_size)
{
    typename Types::ModelBatchOutput output{subbatch_size};
    for (size_t i = 0; i < num_batches; ++i)
    {
        typename Types::ModelBatchInput input{subbatch_size};
        batch_model->inference(input, output);
    }
}

template <typename Types>
double wrap_model_speed_test(
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
        thread_pool[i] = std::thread(thread_test<NewTypes>, &batch_model, num_batches, subbatch_size);
    }
    for (auto &t : thread_pool)
    {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    return (threads * subbatch_size * num_batches) / (double)duration.count() * 1000;
}

int main()
{

    using Types = MoldStateTensorTypes;

    const size_t batch_size = 1 << 8;
    const size_t subbatches = 4;
    const size_t threads = 4;
    const size_t num_batches = 1 << 12;

    Types::Model model{};
    model.to(torch::kCUDA);

    double inf_per_sec = wrap_model_speed_test<Types>(model, batch_size, subbatches, threads, num_batches);
    std::cout << inf_per_sec << std::endl;
    return 0;
}