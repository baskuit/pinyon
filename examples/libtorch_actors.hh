#include <torch/torch.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

using namespace std::chrono_literals;

class TensorActor
{
public:
    TensorActor() : m_stop(false)
    {
        m_thread = std::thread([this]()
                               {
      while (!m_stop) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond_var.wait(lock, [this]() { return !m_queue.empty() || m_stop; });
        if (!m_stop) {
          auto tensor = m_queue.front();
          m_queue.pop_front();
          lock.unlock();
          auto result = processTensor(std::move(tensor));
          lock.lock();
          m_results.push_back(std::move(result));
          m_cond_var.notify_all();
        }
      } });
    }

    ~TensorActor()
    {
        m_stop = true;
        m_cond_var.notify_all();
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    void process(const torch::Tensor &tensor)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push_back(tensor);
        lock.unlock();
        m_cond_var.notify_all();
    }

    bool hasNext()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond_var.wait(lock, [this]()
                        { return !m_results.empty() || m_stop; });
        return !m_results.empty();
    }

    torch::Tensor next()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto result = std::move(m_results.front());
        m_results.pop_front();
        return result;
    }

private:
    std::thread m_thread;
    std::atomic<bool> m_stop;
    std::deque<torch::Tensor> m_queue;
    std::deque<torch::Tensor> m_results;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;

    torch::Tensor processTensor(torch::Tensor tensor)
    {
        // Replace this with your own processing logic
        return tensor * 2;
    }
};

int main()
{
    const int num_actors = 4;
    std::vector<TensorActor> actors(num_actors);

    // Create a vector of input tensors
    std::vector<torch::Tensor> input_tensors;
    for (int i = 0; i < 100; ++i)
    {
        input_tensors.push_back(torch::ones({1, 3, 224, 224}));
    }

    // Process the input tensors using the actors
    std::vector<torch::Tensor> output_tensors;
    output_tensors.reserve(input_tensors.size());
    for (auto &tensor : input_tensors)
    {
        // Select an actor to process the tensor
        auto &actor = actors[std::distance(actors.begin(), std::min_element(actors.begin(), actors.end(), [](const TensorActor &a, const TensorActor &b)
                                                                            { return a.hasNext() < b.hasNext(); }))];

        // Process the tensor using the actor
        actor.process(tensor);
    }

    // Collect the output tensors from the actors
    while (!output_tensors.empty() || std::any_of(actors.begin(), actors.end(), [](const TensorActor &actor)
                                                  { return actor.hasNext(); }))
    {
        for (auto &actor : actors)
        {
            while (actor.hasNext())
            {
                output_tensors.push_back(actor.next());
            }
        }
        std::this_thread::sleep_for(10ms); // Wait a little before checking again
    }
