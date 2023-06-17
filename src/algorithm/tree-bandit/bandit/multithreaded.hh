#pragma once

#include <algorithm/tree-bandit/tree/bandit.hh>

#include <tree/tree.hh>

#include <thread>
#include <mutex>
#include <atomic>

template <class Model, class BanditAlgorithm, 
    template <class> class _Outcome, template <class> class MatrixNode, template <class> class ChanceNode>
class TreeBanditThreaded : public TreeBandit<Model, BanditAlgorithm, _Outcome, MatrixNode, ChanceNode>
{
public:
    struct MatrixStats;
    struct Types : TreeBandit<Model, BanditAlgorithm, _Outcome, MatrixNode, ChanceNode>::Types
    {
        using MatrixStats = TreeBanditThreaded::MatrixStats;
    };

    struct MatrixStats
    {
        std::mutex mtx{};
    };

    int threads = 1;

    // Override the TreeBandit run for threads
    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> &matrix_node)
    {
        this->_initialize_stats(iterations, state, model, &matrix_node);
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

    void runThread(
        const size_t iterations,
        typename Types::PRNG *device,
        typename Types::State *state,
        typename Types::Model *model,
        MatrixNode<BanditAlgorithm> *matrix_node)
    {
        typename Types::PRNG device_thread(device->uniform_64()); // TODO deterministically provide new seed
        typename Types::Model model_thread{*model};
        typename Types::ModelOutput inference;
        for (size_t iteration = 0; iteration < iterations; ++iteration)
        {
            typename Types::State state_copy = *state;
            state_copy.reseed(device_thread.template new_seed<typename Types::Seed>());
            this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
        }
    }

    MatrixNode<BanditAlgorithm> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::ModelOutput &inference)
    {

        std::mutex &mtx = matrix_node->stats.mtx;

        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                mtx.lock();
                this->_select(device, matrix_node, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<BanditAlgorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<BanditAlgorithm> *matrix_node_next = chance_node->access(state.obs);

                MatrixNode<BanditAlgorithm> *matrix_node_leaf = this->run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                mtx.lock();
                this->_update_matrix_node(matrix_node, outcome);
                this->_update_chance_node(chance_node, outcome);
                mtx.unlock();
                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                this->_expand(state, model, matrix_node, inference);
                mtx.unlock();
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

template <class Model, class BanditAlgorithm, 
    template <class> class _Outcome, template <class> class MatrixNode, template <class> class ChanceNode>
class TreeBanditThreadPool : public TreeBandit<Model, BanditAlgorithm, _Outcome, MatrixNode, ChanceNode>
{
public:
    struct MatrixStats;
    struct Types : TreeBandit<Model, BanditAlgorithm, _Outcome, MatrixNode, ChanceNode>::Types
    {
        using MatrixStats = TreeBanditThreadPool::MatrixStats;
    };

    struct MatrixStats
    {
        int mutex_index = 0;
    };

    static constexpr int pool_size = 128;
    int threads = 1;
    std::array<std::mutex, pool_size> mutex_pool;
    std::atomic<int> current_index{0};
    // we simply let this overflow or w/e
    // TODO test overflow behaviour

    void run(
        const size_t iterations,
        typename Types::PRNG &device,
        const typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> &matrix_node)
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

    void runThread(
        const size_t iterations,
        typename Types::PRNG *device,
        typename Types::State *state,
        typename Types::Model *model,
        MatrixNode<BanditAlgorithm> *matrix_node)
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

    MatrixNode<BanditAlgorithm> *run_iteration(
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<BanditAlgorithm> *matrix_node,
        typename Types::ModelOutput &inference)
    {

        std::mutex &mtx = mutex_pool[matrix_node->stats.mutex_index];

        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                mtx.lock();
                this->_select(device, matrix_node, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<BanditAlgorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<BanditAlgorithm> *matrix_node_next = chance_node->access(state.obs);

                MatrixNode<BanditAlgorithm> *matrix_node_leaf = this->run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                mtx.lock();
                this->_update_matrix_node(matrix_node, outcome);
                this->_update_chance_node(chance_node, outcome);
                mtx.unlock();
                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                this->_expand(state, model, matrix_node, inference);
                this->get_mutex_index(matrix_node);
                mtx.unlock();
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
        MatrixNode<BanditAlgorithm> *matrix_node)
    {
        matrix_node->stats.mutex_index = (this->current_index++) % pool_size;
    }
};