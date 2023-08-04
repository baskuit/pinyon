#pragma once

#include <model/model.hh>

#include <torch/torch.h>
#include <condition_variable>
#include <mutex>
#include <atomic>


/*
Thread-safe inference wrapper for Libtorch model.

Needs `n_threads` Threads to call get_inference `n_calls_per_thread` times before model is invoked.
*/

template <class State, class LibtorchModel>
class LibtorchWrapper : public DoubleOracleModel<State>
{
    static_assert(std::derived_from<LibtorchModel, torch::nn::Module>);

public:
    struct Types : DoubleOracleModel<State>::Types
    {
    };

    LibtorchModel *model;
    const int batch_size;
    const int n_threads;
    int n_calls_per_thread;

    torch::Tensor batch_input_tensor = torch::zeros({batch_size, size_of_battle});
    torch::Tensor batch_output_tensor = torch::zeros({batch_size, 1});

    std::atomic<int> ticket{0};
    int seated{0};
    std::vector<int> offsets;
    std::mutex output_mutex;
    std::condition_variable output_cv;

    LibtorchWrapper(
        LibtorchModel *model,
        const int batch_size,
        const int n_threads) : model{model}, batch_size{batch_size}, n_threads{n_calls_per_thread}
    {
        model->eval();
        n_calls_per_thread = batch_size / n_threads;
        offsets.resize(n_threads); // default val is 0
        assert(n_calls_per_thread * n_threads == batch_size);
    }
    void get_inference(int &start_index, torch::Tensor &observation_tesor) // normally would call on a state and matrix_node.
    {
        if (start_index < 0)
        {
            start_index = ticket.fetch_add(n_calls_per_thread);
        }
        else if (start_index >= batch_size)
        {
            return;
        }

        const size_t thread_id = start_index / n_calls_per_thread;
        int &offset = offsets[thread_id];
        const int index = start_index + offset;

        batch_input_tensor[index] = observation_tensor;
        offset += 1;

        if (offset + 1 < n_calls_per_thread)
        {
            return;
        }

        std::unique_lock<std::mutex> output_lock{output_mutex};
        seated += 1;
        const bool is_last_thread_to_finish = (seated == n_threads);

        if (is_last_thread_to_finish)
        {
            seated = 0;
            std::fill(offsets.begin(), offsets.end(), 0);

            batch_output_tensor = model->forward(batch_input_tensor.to(torch::kCUDA)).to(torch::kCPU);

            output_cv.notify_all();
        }
        else
        {
            output_cv.wait(output_lock);
        }
    }

};