#pragma once

#include <algorithm/tree-bandit/tree/tree-bandit.hh>

#include <tree/tree.hh>

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// TODO TODO check speed with read write lock

template <
    class BanditAlgorithm,
    template <class> class MNode = MatrixNode,
    template <class> class CNode = ChanceNode,
    bool return_if_expand = true>
class TreeBanditThreaded : public BanditAlgorithm
{
public:
    struct MatrixStats;
    struct Types : BanditAlgorithm::Types
    {
        using MatrixStats = TreeBanditThreaded::MatrixStats;
        using MatrixNode = MNode<TreeBanditThreaded>;
        using ChanceNode = CNode<TreeBanditThreaded>;
    };

    struct MatrixStats : BanditAlgorithm::MatrixStats
    {
        std::mutex mtx{};
    };

    size_t threads = 1;

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBanditThreaded> &matrix_node)
    {
        this->initialize_stats(iterations, state, model, matrix_node.stats);
        std::thread thread_pool[threads];
        const size_t iterations_per_thread = iterations / threads;
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i] = std::thread(&TreeBanditThreaded::runThread, this, iterations_per_thread, &device, &state, &model, &matrix_node);
        }
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i].join();
        }
    }

    void run_for_duration( // TODO single threaded
        size_t duration_us,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBanditThreaded> &matrix_node)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(end - start);

        while (duration.count() < duration_us)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device.template new_seed<typename Types::Seed>());
            this->run_iteration(device, state_copy, model, &matrix_node);

            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
    }

    void runThread(
        const size_t iterations,
        typename Types::PRNG *device,
        typename Types::State *state,
        typename Types::Model *model,
        MatrixNode<TreeBanditThreaded> *matrix_node)
    {
        typename Types::PRNG device_thread(device->uniform_64()); // TODO deterministically provide new seed
        typename Types::Model model_thread{*model};               // TODO go back to not making new ones? Perhaps only device needs new instance
        typename Types::ModelOutput inference;
        for (size_t iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = *state;
            state_copy.reseed(device_thread.template new_seed<typename Types::Seed>());
            this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
        }
    }

protected:
    MatrixNode<TreeBanditThreaded> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBanditThreaded> *matrix_node,
        typename Types::ModelOutput &inference)
    {
        std::mutex &mtx = matrix_node->stats.mtx;

        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                mtx.lock();
                this->select(device, matrix_node->stats, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<TreeBanditThreaded> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<TreeBanditThreaded> *matrix_node_next = chance_node->access(state.obs);

                MatrixNode<TreeBanditThreaded> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;

                mtx.lock();
                this->update_matrix_stats(matrix_node->stats, outcome);
                this->update_chance_stats(chance_node->stats, outcome);
                mtx.unlock();

                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                state.get_actions();
                matrix_node->row_actions = state.row_actions;
                matrix_node->col_actions = state.col_actions;
                matrix_node->is_expanded = true;
                matrix_node->is_terminal = state.is_terminal;
                typename Types::ModelOutput inference;
                this->expand(state, matrix_node->stats, inference);
                mtx.unlock();

                if (state.is_terminal)
                {
                    inference.value = state.payoff;
                }
                else
                {
                    model.get_inference(state, inference);
                }

                return matrix_node;
            }
        }
        else
        {
            inference.value = state.payoff;
            return matrix_node;
        }
    }
};

/*
TreeBandit with a mutex pool
*/

template <
    class BanditAlgorithm,
    template <class> class MNode,
    template <class> class CNode,
    size_t pool_size = 128,
    bool StopEarly = false>
class TreeBanditThreadPool : public BanditAlgorithm
{
public:
    struct MatrixStats;
    struct Types : BanditAlgorithm::Types
    {
        using MatrixStats = TreeBanditThreadPool::MatrixStats;
        using MatrixNode = MNode<TreeBanditThreadPool>;
        using ChanceNode = CNode<TreeBanditThreadPool>;
    };

    struct MatrixStats
    {
        int mutex_index = 0;
    };

    size_t threads = 1;
    std::array<std::mutex, pool_size> mutex_pool;
    std::atomic<int> current_index{0};
    // we simply let this overflow or w/e
    // TODO test overflow behaviour

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBanditThreadPool> &matrix_node)
    {
        this->_initialize_stats(iterations, state, model, &matrix_node);
        std::thread thread_pool[threads];
        const int iterations_per_thread = iterations / threads;
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i] = std::thread(&TreeBanditThreadPool::runThread, this, iterations_per_thread, &device, &state, &model, &matrix_node);
        }
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i].join();
        }
    }

    void run_for_duration( // TODO single threaded
        size_t duration_us,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBanditThreadPool> &matrix_node)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        while (duration.count() < duration_us)
        {
            typename Types::State state_copy = state;
            state_copy.reseed(device.template new_seed<typename Types::Seed>());
            this->run_iteration(device, state_copy, model, &matrix_node);

            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
    }

    void runThread(
        const size_t iterations,
        typename Types::PRNG *device,
        typename Types::State *state,
        typename Types::Model *model,
        MatrixNode<TreeBanditThreadPool> *matrix_node)
    {
        typename Types::PRNG device_thread{device.template new_seed<typename Types::Seed>()}; // TODO deterministically provide new seed
        typename Types::Model model_thread{*model};
        typename Types::ModelOutput inference;
        for (size_t iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = *state;
            state_copy.reseed(device_thread.template new_seed<typename Types::Seed>());
            this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
        }
    }

    MatrixNode<TreeBanditThreadPool> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<TreeBanditThreadPool> *matrix_node,
        typename Types::ModelOutput &inference)
    {
        std::mutex &mtx = mutex_pool[matrix_node->stats.mutex_index];

        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                mtx.lock();
                select(device, matrix_node->stats, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<TreeBanditThreadPool> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<TreeBanditThreadPool> *matrix_node_next = chance_node->access(state.obs);

                MatrixNode<TreeBanditThreadPool> *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;

                mtx.lock();
                update_matrix_stats(matrix_node->stats, outcome);
                update_chance_stats(chance_node.stats, outcome);
                mtx.unlock();

                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                state.get_actions();
                matrix_node->row_actions = state.row_actions;
                matrix_node->col_actions = state.col_actions;
                matrix_node->is_expanded = true;
                matrix_node->is_terminal = state.is_terminal;
                this->expand(state, matrix_node->stats, inference);
                mtx.unlock();

                if (state.is_terminal)
                {
                    inference.value = state.payoff;
                }
                else
                {
                    model.get_inference(state, inference);
                }

                return matrix_node;
            }
        }
        else
        {
            inference.value = state.payoff;
            return matrix_node;
        }
    }

private:
    void get_mutex_index(
        MatrixNode<TreeBanditThreadPool> *matrix_node)
    {
        matrix_node->stats.mutex_index = (this->current_index++) % pool_size;
    }
};