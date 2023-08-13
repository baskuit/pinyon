#pragma once

#include <algorithm/tree-bandit/tree/tree-bandit.hh>

#include <tree/tree.hh>

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

template <
    CONCEPT(IsMultithreadedBanditTypes, Types),
    template <typename...> typename NodePair = DefaultNodes,
    bool return_if_expand = true>
struct TreeBanditThreaded : Types
{
    struct MatrixStats : Types::MatrixStats
    {
        typename Types::Mutex mtx{};
    };
    struct ChanceStats : Types::ChanceStats
    {
        typename Types::Mutex mtx{};
    };
    using MatrixNode = NodePair<Types, MatrixStats, ChanceStats>::MatrixNode;
    using ChanceNode = NodePair<Types, MatrixStats, ChanceStats>::ChanceNode;

    class Search : public Types::BanditAlgorithm
    {
    public:
        using Types::BanditAlgorithm::BanditAlgorithm;

        Search(Types::BanditAlgorithm &base) : Types::BanditAlgorithm{base} {}

        size_t threads = 1;

        size_t run(
            const size_t duration_ms,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model,
            MatrixNode &matrix_node)
        {
            typename Types::BanditAlgorithm bandit{};
            
            std::thread thread_pool[threads];
            size_t iterations[threads];
            size_t total_iterations = 0;
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i] = std::thread(&Search::run_thread, this, duration_ms, &device, &state, &model, &matrix_node, std::next(iterations, i));
            }
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i].join();
            }
            for (int i = 0; i < threads; ++i)
            {
                total_iterations += iterations[i];
            }
            return total_iterations;
        }

        size_t run_for_iterations(
            const size_t iterations,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model,
            MatrixNode &matrix_node)
        {
            std::thread thread_pool[threads];
            size_t iterations_per_thread = iterations / threads;
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i] = std::thread(&Search::run_thread_for_iterations, this, iterations_per_thread, &device, &state, &model, &matrix_node);
            }
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i].join();
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return duration.count();
        }

    private:
        void run_thread(
            const size_t duration_ms,
            Types::PRNG *device,
            const Types::State *state,
            const Types::Model *model,
            MatrixNode *matrix_node,
            size_t *iterations)
        {
            typename Types::PRNG device_thread(device->uniform_64()); // TODO deterministically provide new seed
            typename Types::Model model_thread{*model};               // TODO go back to not making new ones? Perhaps only device needs new instance
            typename Types::ModelOutput inference;

            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            size_t thread_iterations = 0;
            for (; duration.count() < duration_ms; ++thread_iterations)
            {
                typename Types::State state_copy = *state;
                state_copy.randomize_transition(device_thread);
                this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
                end = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            }
            *iterations = thread_iterations;
        }

        void run_thread_for_iterations(
            const size_t iterations,
            Types::PRNG *device,
            const Types::State *state,
            const Types::Model *model,
            MatrixNode *matrix_node)
        {
            typename Types::PRNG device_thread(device->uniform_64());
            typename Types::Model model_thread{*model};
            typename Types::ModelOutput inference;
            for (size_t iteration = 0; iteration < iterations; ++iteration)
            {
                typename Types::State state_copy = *state;
                state_copy.randomize_transition(device_thread);
                this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
            }
        }

        MatrixNode *run_iteration(
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            MatrixNode *matrix_node,
            Types::ModelOutput &inference)
        {

            typename Types::Mutex &mtx{matrix_node->stats.mtx};

            if (!matrix_node->is_terminal())
            {
                if (!matrix_node->is_expanded())
                {
                    if (state.is_terminal())
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

                // mtx.lock();
                ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                // chance_node->stats.mtx.lock();
                MatrixNode *matrix_node_next = chance_node->access(state.obs);
                // chance_node->stats.mtx.unlock();
                // mtx.unlock();

                MatrixNode *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                this->update_matrix_stats(matrix_node->stats, outcome, mtx);
                this->update_chance_stats(chance_node->stats, outcome, mtx);
                return matrix_node_leaf;
            }
            else
            {
                if constexpr (MatrixNode::STORES_VALUE)
                {
                    matrix_node->get_value(inference.value);
                }
                else
                {
                    inference.value = state.payoff;
                }
                return matrix_node;
            }
        }
    };
};

//  TreeBanditThreadPool

template <
    CONCEPT(IsMultithreadedBanditTypes, Types),
    template <typename...> typename NodePair = DefaultNodes,
    size_t pool_size = 128,
    bool return_if_expand = true>
struct TreeBanditThreadPool : Types
{
    struct MatrixStats : Types::MatrixStats
    {
        int mutex_index = 0;
    };
    struct ChanceStats : Types::ChanceStats
    {
    };
    using MatrixNode = NodePair<Types, MatrixStats, ChanceStats>::MatrixNode;
    using ChanceNode = NodePair<Types, MatrixStats, ChanceStats>::ChanceNode;

    class Search : public Types::BanditAlgorithm
    {
    public:
        using Types::BanditAlgorithm::BanditAlgorithm;

        Search(const Types::BanditAlgorithm &base) : Types::BanditAlgorithm{base} {}

        Search(const Search &other) : Types::BanditAlgorithm{other}, threads{threads} {}
        // we want this class to be copyable, but atomics are not copyable
        // so we define a new copy constr that doesnt attempt to copy it.

        size_t threads = 1;
        std::array<typename Types::Mutex, pool_size> mutex_pool{};
        std::atomic<unsigned int> current_index{0};

        size_t run(
            const size_t duration_ms,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model,
            MatrixNode &matrix_node)
        {
            std::thread thread_pool[threads];
            size_t iterations[threads];
            size_t total_iterations = 0;
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i] = std::thread(&Search::run_thread, this, duration_ms, &device, &state, &model, &matrix_node, std::next(iterations, i));
            }
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i].join();
            }
            for (int i = 0; i < threads; ++i)
            {
                total_iterations += iterations[i];
            }
            return total_iterations;
        }

        size_t run_for_iterations(
            const size_t iterations,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model,
            MatrixNode &matrix_node)
        {
            std::thread thread_pool[threads];
            size_t iterations_per_thread = iterations / threads;
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i] = std::thread(&Search::run_thread_for_iterations, this, iterations_per_thread, &device, &state, &model, &matrix_node);
            }
            for (int i = 0; i < threads; ++i)
            {
                thread_pool[i].join();
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return duration.count();
        }

    private:
        void run_thread(
            const size_t duration_ms,
            Types::PRNG *device,
            const Types::State *state,
            const Types::Model *model,
            MatrixNode *matrix_node,
            size_t *iterations)
        {
            typename Types::PRNG device_thread(device->uniform_64()); // TODO deterministically provide new seed
            typename Types::Model model_thread{*model};               // TODO go back to not making new ones? Perhaps only device needs new instance
            typename Types::ModelOutput inference;

            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            size_t thread_iterations = 0;
            for (; duration.count() < duration_ms; ++thread_iterations)
            {
                typename Types::State state_copy = *state;
                state_copy.randomize_transition(device_thread);
                this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
                end = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            }
            *iterations = thread_iterations;
        }

        void run_thread_for_iterations(
            const size_t iterations,
            Types::PRNG *device,
            const Types::State *state,
            const Types::Model *model,
            MatrixNode *matrix_node)
        {
            typename Types::PRNG device_thread(device->uniform_64());
            typename Types::Model model_thread{*model};
            typename Types::ModelOutput inference;
            for (size_t iteration = 0; iteration < iterations; ++iteration)
            {
                typename Types::State state_copy = *state;
                state_copy.randomize_transition(device_thread);
                this->run_iteration(device_thread, state_copy, model_thread, matrix_node, inference);
            }
        }

        MatrixNode *run_iteration(
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            MatrixNode *matrix_node,
            Types::ModelOutput &inference)
        {
            typename Types::Mutex &mtx = mutex_pool[matrix_node->stats.mutex_index];

            if (!matrix_node->is_terminal())
            {
                if (!matrix_node->is_expanded())
                {
                    if (state.is_terminal())
                    {
                        matrix_node->set_terminal();
                        inference.value = state.payoff;
                    }
                    else
                    {
                        state.get_actions();
                        model.get_inference(state, inference);
                        get_mutex_index(matrix_node);

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

                // mtx.lock();
                ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                // chance_node->stats.mtx.lock();
                MatrixNode *matrix_node_next = chance_node->access(state.obs);
                // chance_node->stats.mtx.unlock();
                // mtx.unlock();

                MatrixNode *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, inference);

                outcome.value = inference.value;
                this->update_matrix_stats(matrix_node->stats, outcome, mtx);
                this->update_chance_stats(chance_node->stats, outcome, mtx);
                return matrix_node_leaf;
            }
            else
            {
                if constexpr (MatrixNode::STORES_VALUE)
                {
                    matrix_node->get_value(inference.value);
                }
                else
                {
                    inference.value = state.payoff;
                }
                return matrix_node;
            }
        }

    private:
        void get_mutex_index(
            MatrixNode *matrix_node)
        {
            matrix_node->stats.mutex_index = (this->current_index.fetch_add(1)) % pool_size;
        }
    };
};