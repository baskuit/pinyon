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
    using ModelBatchOutput = Types::ModelBatchOutput;
    using ModelBatchInput = Types::ModelBatchInput;

    struct ModelLock
    {
        ModelLock(
            const long int batch_size,
            const long int subbatches)
            : batch_size{batch_size},
              subbatches{subbatches},
              subbatch_size{batch_size / subbatches},
              combined_batch_input{batch_size},
              combined_batch_output{batch_size}
        {
            for (size_t sb = 0; sb < subbatches; ++sb)
            {
                seats.push(sb);
            }
        };

        const long int batch_size;
        const long int subbatches;
        const long int subbatch_size;

        ModelBatchInput combined_batch_input;
        ModelBatchOutput combined_batch_output;

        std::queue<size_t> seats{};
        size_t seated = 0;

        std::mutex input_mutex;
        std::condition_variable input_cv;

        std::mutex output_mutex;
        std::condition_variable output_cv;

        void inference(
            Types::Model &model,
            ModelBatchInput &model_batch_input,
            ModelBatchOutput &model_batch_output)
        {
            size_t subbatch_id;
            std::unique_lock input_lock{input_mutex};

            if (seats.size() == 0)
            {
                input_cv.wait(input_lock);
                subbatch_id = seats.front();
                seats.pop();
            }
            else
            {
                subbatch_id = seats.front();
                seats.pop();
                input_lock.unlock();
            }

            const size_t subbatch_index = subbatch_size * subbatch_id;
            combined_batch_input.copy_range(model_batch_input, subbatch_index, subbatch_index + subbatch_size);

            std::unique_lock<std::mutex> output_lock{output_mutex};
            const bool is_last_thread_to_finish = (++seated == subbatches);

            if (is_last_thread_to_finish)
            {
                seated = 0;
                model.inference(combined_batch_input, combined_batch_output);
                output_cv.notify_all();
            }
            else
            {
                output_cv.wait(output_lock);
            }

            input_mutex.lock();
            seats.push(subbatch_id);
            input_mutex.unlock();
            input_cv.notify_one();

            combined_batch_output.copy_range(model_batch_output, subbatch_index, subbatch_index + subbatch_size);
        }
    };

    class Model : public Types::Model
    {
    public:
        std::shared_ptr<ModelLock> synch_mechanism = nullptr;

        template <typename... Args>
        Model(const Args &...args, const long int batch_size, const long int subbatches) : Types::Model{args...}
        {
            synch_mechanism = std::make_shared<ModelLock>(batch_size, subbatches);
        }

        Model(const Types::Model &model, const long int batch_size, const long int subbatches) : Types::Model{model}
        {
            synch_mechanism = std::make_shared<ModelLock>(batch_size, subbatches);
        }

        void inference(
            ModelBatchInput &model_batch_input,
            ModelBatchOutput &model_batch_output)
        {
            model_batch_output = ModelBatchOutput{synch_mechanism->batch_size};
            synch_mechanism->inference(*this, model_batch_input, model_batch_output);
        }
    };
};
