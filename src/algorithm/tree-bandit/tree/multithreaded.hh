#pragma once

#include <algorithm/tree-bandit/tree/tree-bandit.hh>

#include <tree/tree.hh>

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

template <
    class BanditAlgorithm,
    template <class> class MNode = MatrixNode,
    template <class> class CNode = ChanceNode,
    bool return_if_expand = true>
class TreeBanditThreaded : public BanditAlgorithm
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : BanditAlgorithm::Types
    {
        using MatrixStats = TreeBanditThreaded::MatrixStats;
        using ChanceStats = TreeBanditThreaded::ChanceStats;
        using MatrixNode = MNode<TreeBanditThreaded>;
        using ChanceNode = CNode<TreeBanditThreaded>;
    };

    struct MatrixStats : BanditAlgorithm::MatrixStats
    {
        std::mutex mtx{};
    };
    struct ChanceStats : BanditAlgorithm::ChanceStats
    {
        std::mutex mtx{};
    };

    size_t threads = 1;

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MNode<TreeBanditThreaded> &matrix_node)
    {
        this->initialize_stats(iterations, state, model, matrix_node.stats);
        std::thread thread_pool[threads];
        const size_t iterations_per_thread = iterations / threads;
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i] = std::thread(&TreeBanditThreaded::run_thread, this, iterations_per_thread, &device, &state, &model, &matrix_node);
        }
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i].join();
        }
    }

    void run_for_duration(
        const size_t duration_us,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MNode<TreeBanditThreaded> &matrix_node)
    {
        // this->initialize_stats(iterations, state, model, matrix_node.stats);
        std::thread thread_pool[threads];
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i] = std::thread(&TreeBanditThreaded::run_thread_for_duration, this, duration_us, &device, &state, &model, &matrix_node);
        }
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i].join();
        }
    }

private:
    void run_thread(
        const size_t iterations,
        typename Types::PRNG *device,
        const typename Types::State *state,
        const typename Types::Model *model,
        MNode<TreeBanditThreaded> *matrix_node)
    {
        typename Types::PRNG device_thread(device->uniform_64()); // TODO deterministically provide new seed
        typename Types::Model model_thread{*model};               // TODO go back to not making new ones? Perhaps only device needs new instance
        typename Types::ModelOutput inference;

        for (size_t iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = *state;
            state_copy.reseed(device_thread);
            this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
        }
    }

    void run_thread_for_duration(
        const size_t duration_us,
        typename Types::PRNG *device,
        const typename Types::State *state,
        const typename Types::Model *model,
        MNode<TreeBanditThreaded> *matrix_node)
    {
        typename Types::PRNG device_thread(device->uniform_64());
        typename Types::Model model_thread{*model};
        typename Types::ModelOutput inference;

        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(end - start);

        while (duration.count() < duration_us)
        {
            typename Types::State state_copy = *state;
            state_copy.reseed(device_thread);
            this->run_iteration(device_thread, state_copy, model_thread, &matrix_node, inference);

            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
    }

    MNode<TreeBanditThreaded> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MNode<TreeBanditThreaded> *matrix_node,
        typename Types::ModelOutput &inference)
    {

        std::mutex &mtx{matrix_node->stats.mtx};

        if (!matrix_node->is_terminal())
        {
            if (!matrix_node->is_expanded())
            {
                if (state.is_terminal)
                {
                    matrix_node->set_terminal();
                    inference.value = state.payoff;
                }
                else
                {
                    state.get_actions();
                    model.get_inference(state, inference);

                    mtx.lock();
                    matrix_node->expand(state);
                    this->expand(state, matrix_node->stats, inference);
                    mtx.unlock();
                }

                if constexpr (return_if_expand)
                {
                    return matrix_node;
                }
            }

            typename Types::Outcome outcome;
            this->select(device, matrix_node->stats, outcome, mtx);

            matrix_node->apply_actions(state, outcome.row_idx, outcome.col_idx);

            CNode<TreeBanditThreaded> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
            // chance_node->stats.mtx.lock();
            MNode<TreeBanditThreaded> *matrix_node_next = chance_node->access(state.obs);
            // chance_node->stats.mtx.unlock();

            MNode<TreeBanditThreaded> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

            outcome.value = inference.value;
            this->update_matrix_stats(matrix_node->stats, outcome, mtx);
            this->update_chance_stats(chance_node->stats, outcome, mtx);
            return matrix_node_leaf;
        }
        else
        {
            inference.value = state.payoff;
            return matrix_node;
        }
    }
};

//  TreeBanditThreadPool

template <
    class BanditAlgorithm,
    template <class> class MNode,
    template <class> class CNode,
    size_t pool_size = 128,
    bool return_if_expand = false>
class TreeBanditThreadPool : public BanditAlgorithm
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : BanditAlgorithm::Types
    {
        using MatrixStats = TreeBanditThreadPool::MatrixStats;
        using ChanceStats = TreeBanditThreadPool::ChanceStats;
        using MatrixNode = MNode<TreeBanditThreadPool>;
        using ChanceNode = CNode<TreeBanditThreadPool>;
    };

    struct MatrixStats : BanditAlgorithm::MatrixStats
    {
        int mutex_index = 0;
    };
    struct ChanceStats : BanditAlgorithm::ChanceStats
    {
    };

    size_t threads = 1;
    std::array<std::mutex, pool_size> mutex_pool;
    std::atomic<unsigned int> current_index{0};

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MNode<TreeBanditThreadPool> &matrix_node)
    {
        this->initialize_stats(iterations, state, model, matrix_node.stats);
        std::thread thread_pool[threads];
        const size_t iterations_per_thread = iterations / threads;
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i] = std::thread(&TreeBanditThreadPool::run_thread, this, iterations_per_thread, &device, &state, &model, &matrix_node);
        }
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i].join();
        }
    }

    void run_thread_for_duration(
        const size_t duration_us,
        typename Types::PRNG *device,
        const typename Types::State *state,
        const typename Types::Model *model,
        MNode<TreeBanditThreadPool> *matrix_node)
    {
        typename Types::PRNG device_thread(device->uniform_64());
        typename Types::Model model_thread{*model};
        typename Types::ModelOutput inference;

        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(end - start);

        while (duration.count() < duration_us)
        {
            typename Types::State state_copy = *state;
            state_copy.reseed(device_thread);
            this->run_iteration(device_thread, state_copy, model_thread, &matrix_node, inference);

            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
    }

private:
    void run_thread(
        const size_t iterations,
        typename Types::PRNG *device,
        const typename Types::State *state,
        const typename Types::Model *model,
        MNode<TreeBanditThreadPool> *matrix_node)
    {
        typename Types::PRNG device_thread{device.template new_seed<typename Types::Seed>()};
        typename Types::Model model_thread{*model};
        typename Types::ModelOutput inference;
        for (size_t iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = *state;
            state_copy.reseed(device_thread.template new_seed<typename Types::Seed>());
            this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
        }
    }

    MNode<TreeBanditThreadPool> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MNode<TreeBanditThreadPool> *matrix_node,
        typename Types::ModelOutput &inference)
    {
        std::mutex &mtx = mutex_pool[matrix_node->stats.mutex_index];

        if (!matrix_node->is_terminal())
        {
            if (!matrix_node->is_expanded())
            {
                if (state.is_terminal)
                {
                    matrix_node->set_terminal();
                    inference.value = state.payoff;
                }
                else
                {
                    state.get_actions();
                    model.get_inference(state, inference);

                    mtx.lock();
                    matrix_node->expand(state);
                    this->expand(state, matrix_node->stats, inference);
                    mtx.unlock();
                }

                if constexpr (return_if_expand)
                {
                    return matrix_node;
                }
            }

            typename Types::Outcome outcome;
            this->select(device, matrix_node->stats, outcome, mtx);

            matrix_node->apply_actions(state, outcome.row_idx, outcome.col_idx);

            CNode<TreeBanditThreadPool> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
            MNode<TreeBanditThreadPool> *matrix_node_next = chance_node->access(state.obs);

            MNode<TreeBanditThreadPool> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

            outcome.value = inference.value;
            this->update_matrix_stats(matrix_node->stats, outcome, mtx);
            this->update_chance_stats(chance_node->stats, outcome, mtx);
            return matrix_node_leaf;
        }
        else
        {
            inference.value = state.payoff;
            return matrix_node;
        }
    }

private:
    void get_mutex_index(
        MNode<TreeBanditThreadPool> *matrix_node)
    {
        matrix_node->stats.mutex_index = (this->current_index.fetch_add(1)) % pool_size;
    }
};