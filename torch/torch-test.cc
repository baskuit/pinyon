#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <array>
#include <atomic>
#include <tuple>
#include <algorithm>

#include <torch/torch.h>
#include "wrapper0.hh"
#include "wrapper1.hh"
#include "net.hh"
#include "utils.hh"

#include "state/random-tree.hh"
#include "model/model.hh"
#include "algorithm/bandits/exp3.hh"
#include "tree/tree.hh"

std::vector<size_t> range(const size_t a, const size_t b, const size_t c = 1)
{
    std::vector<size_t> x{};
    for (size_t i = a; i < b; i += c)
    {
        x.push_back(i);
    }
    return x;
}

template <class Wrapper>
void thread_loop_inference(Wrapper *wrapper, size_t num_batches_to_process, int subbatch_size)
{

    /*
    One threads task: make matrix nodes, inference, add data to matrix nodes, cleanup.
    Kinda simulates tree search
    Doing this on a tree saerch coming sone
    */

    using MatrixNode = MatrixNode<Exp3<MonteCarloModel<RandomTree>, TreeBandit>>;
    std::vector<MatrixNode *> matrix_node_vector{};

    torch::Tensor thread_input_tensor = torch::zeros({subbatch_size, size_of_battle});
    torch::Tensor thread_output_tensor = torch::zeros({subbatch_size, 1});

    for (size_t i = 0; i < num_batches_to_process; ++i)
    {

        for (size_t b = 0; b < subbatch_size; ++b)
        {
            MatrixNode *matrix_node = new MatrixNode();
            matrix_node_vector.push_back(matrix_node);
        }

        wrapper->inference(thread_input_tensor, thread_output_tensor);

        for (size_t b = 0; b < subbatch_size; ++b)
        {
            auto &matrix_node = matrix_node_vector[b];
            auto &inference = matrix_node->inference;
            inference.row_value = thread_output_tensor[b]. template item<double>();
            inference.col_value = 1 - matrix_node->inference.row_value;
            delete matrix_node;
        }

        matrix_node_vector.clear();
    }
};

template <class Wrapper, class Model>
void grid_search_benchmark(
    std::vector<std::tuple<double, int, int>> &data,
    size_t iterations,
    std::vector<size_t> &log2_batch_size,
    std::vector<size_t> &log2_threads,
    Model *model)
{
    for (const auto b : log2_batch_size)
    {
        const int batch_size = 1 << b;
        for (const auto t : log2_threads)
        {
            int n_threads = 1 << t;
            const int subbatch_size = batch_size / n_threads;
            const int total_samples = batch_size * iterations;

            n_threads += 0;

            Wrapper wrapper(batch_size, n_threads, model);

            auto start_time = std::chrono::steady_clock::now();

            // wrapper.max_speed_test(iterations, batch_size);

            std::vector<std::thread> thread_vector;
            for (size_t thread = 0; thread < n_threads; ++thread)
            {
                thread_vector.emplace_back(&thread_loop_inference<Wrapper>, &wrapper, iterations, subbatch_size);
            }
            for (auto &thread : thread_vector)
            {
                thread.join();
            }

            auto end_time = std::chrono::steady_clock::now();

            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            double samples_per_second = static_cast<double>(total_samples) / (static_cast<double>(elapsed_time) / 1000.0);

            std::tuple<double, int, int> datum{samples_per_second, batch_size, n_threads};
            data.push_back(datum);
            // double x = torch::cuda::max_memory_allocated();

            std::cout << samples_per_second << ' ' << n_threads << ' ' << batch_size << std::endl;
        }
    }
}

int main()
{
    if (!torch::cuda::is_available())
    {
        std::cerr << "CUDA is not available. Exiting..." << std::endl;
        return 1;
    }

    using Model = TwoLayerMLP;
    using InferenceWrapper = InferenceWrapper1<Model>;

    // Model model(6, 128);
    Model model;
    model.to(torch::kCUDA);

    const int n_batches_to_process = 1000;
    auto log2_batch_size = range(5, 10);
    auto log2_threads = range(0, 4);

    std::vector<std::tuple<double, int, int>> data{};

    grid_search_benchmark<InferenceWrapper, Model>(data, n_batches_to_process, log2_batch_size, log2_threads, &model);

    std::sort(data.begin(), data.end(), [](const std::tuple<double, int, int> &a, const std::tuple<double, int, int> &b)
              {
                  return std::get<0>(a) > std::get<0>(b); // Compare the first tuple elements
              });

    std::ofstream file("tests/benchmark.txt", std::ios::app);

    auto now = system_clock::now();
    auto now_time = system_clock::to_time_t(now);
    file << "Test date/time: " << std::put_time(std::localtime(&now_time), "%F %T") << "\n";
    file << "samples/sec; batch size, threads" << std::endl;
    for (const auto &tuple : data)
    {
        file << std::get<0>(tuple) << ", " << std::get<1>(tuple) << ", " << std::get<2>(tuple) << std::endl;
    }

    return 0;
}