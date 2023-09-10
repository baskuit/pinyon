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

void printTensorSize(const torch::Tensor& tensor) {
    auto size = tensor.sizes();
    for (int64_t dim : size) {
        std::cout << dim << " ";
    }
    std::cout << std::endl;
}

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

        torch::Tensor batch_input_tensor = this->get_random_input(batch_size);
        torch::Tensor batch_output_tensor = this->get_random_output(batch_size);

        std::queue<size_t> seats{}; // init: {0, 1, 2, ..., minibatches - 1}
        size_t seated{0};           // number of threads that have finished copying minibatch input to batch input
        // lock to secure minibatch
        std::mutex input_mutex;
        std::condition_variable input_cv;
        // lock to stop new inference before all threads have gathered minibatch output
        std::mutex output_mutex;
        std::condition_variable output_cv;

        void inference(
            ModelBatchInput &model_batch_input,
            ModelBatchOutput &model_batch_output)
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
            batch_input_tensor.index_put_({torch::indexing::Slice(minibatch_index, minibatch_index + minibatch_size)}, model_batch_input);

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

            // gather minibatch output size_to model_batch_output
            // Do this after freeing input_tensor, so that new thread can 'enter' while the old thread 'leaves'
            // TODO lol
            // model_batch_output.index_put_({torch::indexing::Slice(minibatch_index, minibatch_index + minibatch_size)}, batch_output_tensor.index({torch::indexing::Slice(0, 128)}));
            model_batch_output.copy_(batch_output_tensor.index({torch::indexing::Slice(minibatch_index, minibatch_index + minibatch_size)}));
        }
    };

    using Model = ModelWithSync;
};
