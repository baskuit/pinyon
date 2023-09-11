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

    struct ModelLock
    {
        ModelLock(
            const long int batch_size,
            const long int minibatches)
            : batch_size{batch_size},
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

        std::queue<size_t> seats{};
        size_t seated = 0;

        std::mutex input_mutex;
        std::condition_variable input_cv;

        std::mutex output_mutex;
        std::condition_variable output_cv;

        torch::Tensor input_tensor;
        torch::Tensor output_tensor;

        void inference(
            Types::Model &model,
            ModelBatchInput &model_batch_input,
            ModelBatchOutput &model_batch_output)
        {
            size_t minibatch_id;
            std::unique_lock input_lock{input_mutex};

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

            const size_t minibatch_index = minibatch_size * minibatch_id;
            input_tensor.index_put_({torch::indexing::Slice(minibatch_index, minibatch_index + minibatch_size)}, model_batch_input);

            std::unique_lock<std::mutex> output_lock{output_mutex};
            const bool is_last_thread_to_finish = (++seated == minibatches);

            if (is_last_thread_to_finish)
            {
                seated = 0;
                output_tensor = model.forward(input_tensor.to(torch::kCUDA)).to(torch::kCPU);
                output_cv.notify_all();
            }
            else
            {
                output_cv.wait(output_lock);
            }

            input_mutex.lock();
            seats.push(minibatch_id);
            input_mutex.unlock();
            input_cv.notify_one();

            model_batch_output.copy_(output_tensor.index({torch::indexing::Slice(minibatch_index, minibatch_index + minibatch_size)}));
        }
    };

    class Model : public Types::Model
    {
    public:
        std::shared_ptr<ModelLock> synch_mechanism = nullptr;

        template <typename... Args>
        Model(const Args &...args, const size_t batch_size, const size_t subbatches) :
        Types::Model{args...}
        {
            synch_mechanism = std::make_shared<ModelLock>(batch_size, subbatches);
            synch_mechanism->input_tensor = this->get_random_input(batch_size);
            synch_mechanism->output_tensor = this->get_random_output(batch_size);
        }

        void inference(
            ModelBatchInput &model_batch_input,
            ModelBatchOutput &model_batch_output)
        {
            synch_mechanism->inference(*this, model_batch_input, model_batch_output);
        }
    };
};
