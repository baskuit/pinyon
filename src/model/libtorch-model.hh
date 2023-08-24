#pragma once

#include <model/model.hh>

#include <torch/torch.h>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <atomic>

template <size_t size>
struct MoldStateLibtorchModel : MoldState<size> {

    using ModelInput = typename Types::State;
    struct ModelOutput {
        typename Types::Value value;
    };
    using BatchModelInput = torch::Tensor;
    using BatchModelOutput = torch::Tensor;

    class Model {
    public:

        // void get_input (
        //     Types::State &state,
        //     Types::ModelInput &model_input
        // ) {

        // }

        void inference (
            BatchModelInput &batch_model_input,
            BatchModelOutput &batch_model_output
        ) {

        }

        void add_to_batch_input (
            Types::State &state,
            BatchModelInput &batch_model_input
        ) {

        }

    };    

};


/*
Thread-safe inference wrapper for Libtorch model.

Needs `n_threads` Threads to call inference `n_calls_per_thread` times before model is invoked.
*/

template <class LibtorchModel>
class InferenceWrapper1
{

public:

    LibtorchModel *model;

    const int batch_size;
    const int minibatches;
    int minibatch_size;

    torch::Tensor batch_input_tensor = torch::zeros({batch_size, size_of_battle});
    torch::Tensor batch_output_tensor = torch::zeros({batch_size, 1});

    std::queue<int> seats{}; // init: {0, 1, 2, ..., minibatches - 1}
    int seated{0}; // number of threads that have finished copying minibatch input to batch input

    // lock to secure minibatch
    std::mutex input_mutex;
    std::condition_variable input_cv;

    // lock to stop new inference before all threads have gathered minibatch output
    std::mutex output_mutex;
    std::condition_variable output_cv;

    void get_inference(torch::Tensor &thread_input_tensor, torch::Tensor &thread_output_tensor) // normally would call on a state and matrix_node.
    {

        /*
        thread_input_tensor:
            minibatch of obs tensors that a thread has collected from its trees
        thread_output_tensor:
            minibatch inference output
        */

        // FAKE WORK: Populate minibatch input
        // NORMALLY: Done by thread before calling get_inference()
        thread_input_tensor = torch::empty({minibatch_size, size_of_battle});
        for (size_t i = 0; i < minibatch_size; ++i) {
            const auto x = torch::rand({size_of_battle});
            thread_input_tensor[i] = x;
        }

        // Thread owns the Slice [minibatch_id * minibatch_size, (minibatch_id + 1) * minibatch_size]
        // Don't have to lock guard tensors this way
        size_t minibatch_id;

        std::unique_lock input_lock{input_mutex};

        // wait for minibatch slice to open
        // `wait` will cease after other thread has gathered output and added new index to queue
        if (seats.size() == 0)
        {
            input_cv.wait(input_lock);
            minibatch_id = seats.front();
            seats.pop();
        }
        else
        {
            minibatch_id = seats.front();
            seats.pop();
            input_lock.unlock();
        }

        // write input to secured minibatch slice
        const int minibatch_index = minibatch_size * minibatch_id;
        batch_input_tensor.index_put_({torch::indexing::Slice(minibatch_index, minibatch_index + minibatch_size)}, thread_input_tensor);

        // prepare to wait for inference
        std::unique_lock<std::mutex> output_lock{output_mutex};
        const bool is_last_thread_to_finish = (++seated == minibatches);

        if (is_last_thread_to_finish)
        {

            // reset input completion stuff
            seated = 0;
            // send to GPU
            batch_output_tensor = model->forward(batch_input_tensor.to(torch::kCUDA)).to(torch::kCPU);
            // wake the earlier threads up
            output_cv.notify_all();
        }
        else
        {
            // early threads wait here
            output_cv.wait(output_lock);
        }

        // now that inference done, add minibatch_id back to queue so input can be overwritten by new thread
        input_mutex.lock();
        seats.push(minibatch_id);
        input_mutex.unlock();
        input_cv.notify_one();

        // gather minibatch output into thread_output_tensor
        // Do this after freeing input_tensor, so that new thread can 'enter' while the old thread 'leaves'
        // TODO lol
    }

    InferenceWrapper1(const int batch_size, const int minibatches, LibtorchModel *model) : batch_size{batch_size}, minibatches{minibatches}, model{model}
    {
        for (int sb = 0; sb < minibatches; ++sb)
        {
            seats.push(sb);
        }
        minibatch_size = batch_size / minibatches;
    };
};