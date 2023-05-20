#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <array>
#include <atomic>

class Batch {
public:
    static const size_t batch_size = 2;
    std::queue<int> input_queue;
    std::array<int, batch_size> output_pool;
    std::mutex input_mutex;
    std::condition_variable cv;
    std::atomic<bool> done{ false };

    int get_inference(int input) {
        std::cout << "Start" << std::endl;

        size_t input_idx;
        std::mutex local_mutex;
        std::unique_lock<std::mutex> local_lock(local_mutex);
        
        input_mutex.lock();
        input_idx = input_queue.size();
        input_queue.emplace(input);

        if (input_queue.size() >= batch_size) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (size_t idx = 0; idx < batch_size; ++idx) {
                int input = input_queue.front();
                input_queue.pop();
                int output = input + 1;
                output_pool[idx] = output;
            }
            done.store(true);
            cv.notify_all();
        }

        input_mutex.unlock();
        std::cout << "Unlocked input" << std::endl;
        cv.wait(local_lock, [this] { return this->done.load(); });
        int output = output_pool[input_idx];

        std::cout << "Input: " << input << " Output: " << output << std::endl;
        // std::cout.flush();
        return output;
    }
};

int main() {
    Batch x;

    size_t threads = 32;
    std::vector<std::thread> pool;
    for (size_t thread_dx = 0; thread_dx < threads; ++thread_dx) {
        pool.emplace_back(&Batch::get_inference, &x, thread_dx);
    }

    // Join all the threads in the pool
    for (auto& thread : pool) {
        thread.join();
    }

    return 0;
}
