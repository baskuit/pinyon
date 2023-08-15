#pragma once

#include <iostream>
#include <condition_variable>
#include <mutex>
#include <array>
#include <atomic>

#include <torch/torch.h>
#include "utils.hh"

template <class LibtorchModel>
class InferenceWrapper0
{

public:
    const int batch_size;
    const int subbatches;
    int subbatch_size;
    std::vector<int> offsets;
    LibtorchModel *model;

    // fixed input tensor that is fed to model in one shot
    torch::Tensor batch_input_tensor = torch::zeros({batch_size, size_of_battle});
    // model output copied here for processing
    torch::Tensor batch_output_tensor = torch::zeros({batch_size, 1});

    // wrapper will only allow you to ride if you have ticket
    // this only works if at least subbatches many threads are calling inference
    // if any more threads call, their ticket number is too high to ride
    std::atomic<int> ticket{0};

    // count for how many threads have written to input and are now waiting
    std::atomic<int> seated{0};

    // condition variable that lets seated threads no to gather their output
    std::mutex output_mutex;
    std::condition_variable output_cv;

    InferenceWrapper0(const int batch_size, const int subbatches, LibtorchModel *model) : batch_size{batch_size}, subbatches{subbatches}, model{model}
    {
        subbatch_size = batch_size / subbatches;
        offsets.resize(subbatches);
    };

    void inference(int &start_index) // normally would call on a state and matrix_node.
    {

        /*
        Normally this function takes a State and a MatrixNode. The state's values are estimated and written to MatrixNode::Inference
        This is only meant to simulate for benchmarking so instead we just randomly generate input tensor with same number of bits as a battle
        This function only handles filling input and running model.
        Processing of output by the threads is done outside.
        */

        torch::Tensor observation_tensor = torch::rand({size_of_battle});

        // threads call with start_index = -1 at first, to indicate they don't have a chunk [start_index, start_index + subbatch_size] reserved yet
        if (start_index < 0)
        {
            start_index = ticket.fetch_add(subbatch_size);
        }
        else if (start_index >= batch_size)
        {
            return; // too late, too many threads
        }

        // wrapper stores chunk progress/offset for each thread
        const size_t thread_id = start_index / subbatch_size;
        int &offset = offsets[thread_id];
        const int index = start_index + offset;

        // write to chunk, increment for next inference() call
        batch_input_tensor[index] = observation_tensor;
        offset += 1;

        // if you stil have chunk to write too, then proceed
        if (offset + 1 < subbatch_size)
        {
            return;
        }

        std::unique_lock<std::mutex> output_lock{output_mutex};
        const bool is_last_thread_to_finish = (seated.fetch_add(1) + 1 == subbatches); // remove atomic, using mutex now

        if (is_last_thread_to_finish)
        {

            // reset input completion stuff
            seated.store(0);
            std::fill(offsets.begin(), offsets.end(), 0);

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
    }
};