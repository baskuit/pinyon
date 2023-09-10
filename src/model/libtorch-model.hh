#pragma once

#include <model/model.hh>

#include <torch/torch.h>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <atomic>

/*
Thread-safe inference wrapper for Libtorch model.

Needs `n_threads` Threads to call inference `n_calls_per_thread` times before model is invoked.
*/

template <CONCEPT(IsPerfectInfoStateTypes, Types)>
struct LibtorchBatchModel : Types
{
    using ModelBatchOutput = torch::Tensor;
    using ModelBatchInput = torch::Tensor;

    class ModelWithSync : public Types::Model
    {
    public:
        ModelWithSync(
            const Types::Model &model,
            const long int batch_size,
            const long int minibatches)
            : Types::Model{model},
              batch_size{batch_size},
              minibatches{minibatches},
              minibatch_size{batch_size / minibatches}
        {
            for (size_t sb = 0; sb < minibatches; ++sb)
            {
                seats.push(sb);
            }
        };

        const long int batch_size;
        const long int minibatches;
        const long int minibatch_size;

        torch::Tensor batch_input_tensor = torch::zeros({batch_size, 386});
        torch::Tensor batch_output_tensor = torch::zeros({batch_size, 1});

        std::queue<size_t> seats{}; // init: {0, 1, 2, ..., minibatches - 1}
        size_t seated{0};           // number of threads that have finished copying minibatch input to batch input
        // lock to secure minibatch
        std::mutex input_mutex;
        std::condition_variable input_cv;
        // lock to stop new inference before all threads have gathered minibatch output
        std::mutex output_mutex;
        std::condition_variable output_cv;

        void inference(
            ModelBatchInput &thread_input_tensor,
            ModelBatchOutput &thread_output_tensor)
        {
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
            const size_t minibatch_index = minibatch_size * minibatch_id;
            batch_input_tensor.index_put_({torch::indexing::Slice(minibatch_index, minibatch_index + minibatch_size)}, thread_input_tensor);

            // prepare to wait for inference
            std::unique_lock<std::mutex> output_lock{output_mutex};
            const bool is_last_thread_to_finish = (++seated == minibatches);

            if (is_last_thread_to_finish)
            {
                // reset input completion stuff
                seated = 0;
                batch_output_tensor = this->forward(batch_input_tensor.to(torch::kCUDA)).to(torch::kCPU);
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

            // gather minibatch output size_to thread_output_tensor
            // Do this after freeing input_tensor, so that new thread can 'enter' while the old thread 'leaves'
            // TODO lol
        }
    };

    using Model = ModelWithSync;
};
