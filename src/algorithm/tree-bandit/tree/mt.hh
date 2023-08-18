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
struct TreeBanditThreaded2 : Types
{
    struct MatrixStats : Types::MatrixStats
    {
        typename Types::Mutex mutex{};
        typename Types::Mutex mutex_{};
    };
    struct ChanceStats : Types::ChanceStats
    {
        typename Types::Mutex mutex{};
    };
    using MatrixNode = NodePair<Types, MatrixStats, ChanceStats>::MatrixNode;
    using ChanceNode = NodePair<Types, MatrixStats, ChanceStats>::ChanceNode;

    class Search : public Types::BanditAlgorithm
    {
    public:
        using Types::BanditAlgorithm::BanditAlgorithm;

        Search(const Types::BanditAlgorithm &base) : Types::BanditAlgorithm{base}
        {
        }

        Search(const Types::BanditAlgorithm &base, size_t threads) : Types::BanditAlgorithm{base}, threads{threads}
        {
        }

        const size_t threads = 1;

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
                thread_pool[i] = std::thread(
                    &Search::run_thread, this, duration_ms, device.uniform_64(), &state, &model, &matrix_node, std::next(iterations, i));
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
                thread_pool[i] = std::thread(
                    &Search::run_thread_for_iterations, this, iterations_per_thread, device.uniform_64(), &state, &model, &matrix_node);
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
            const Types::Seed thread_device_seed,
            const Types::State *state,
            const Types::Model *model,
            MatrixNode *matrix_node,
            size_t *iterations)
        {
            typename Types::PRNG device_thread(thread_device_seed);
            typename Types::Model model_thread{*model};
            typename Types::ModelOutput model_output;

            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            size_t thread_iterations = 0;
            for (; duration.count() < duration_ms; ++thread_iterations)
            {
                typename Types::State state_copy = *state;
                state_copy.randomize_transition(device_thread);
                this->run_iteration(device_thread, state_copy, model_thread, matrix_node, model_output);
                end = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            }
            *iterations = thread_iterations;
        }

        void run_thread_for_iterations(
            const size_t iterations,
            const Types::Seed thread_device_seed,
            const Types::State *state,
            const Types::Model *model,
            MatrixNode *const matrix_node)
        {
            typename Types::PRNG device_thread(thread_device_seed);
            typename Types::Model model_thread{*model};
            typename Types::ModelOutput model_output;
            for (size_t iteration = 0; iteration < iterations; ++iteration)
            {
                typename Types::State state_copy = *state;
                state_copy.randomize_transition(device_thread);
                this->run_iteration(device_thread, state_copy, model_thread, matrix_node, model_output);
            }
        }

        MatrixNode *run_iteration(
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            MatrixNode *matrix_node,
            Types::ModelOutput &model_output)
        {
            typename Types::Mutex &mutex{matrix_node->stats.mutex};
            typename Types::Mutex &mutex_{matrix_node->stats.mutex_};

            // no longer allows for artifically terminal nodes
            if (state.is_terminal())
            {
                matrix_node->set_terminal();
                model_output.value = state.payoff;
                return matrix_node;
            }
            else
            {
                if (!matrix_node->is_expanded())
                {
                    if (mutex.try_lock())
                    {
                        if (matrix_node->is_expanded())
                        {
                            mutex.unlock();
                            return matrix_node;
                            // double exapands may throw
                        }
                        // gets to expand
                        // state.get_actions(); // alread has em
                        model.inference(state, model_output);
                        this->expand(state, matrix_node->stats, model_output);
                        mutex.unlock();
                        matrix_node->expand(state);
                    }
                    return matrix_node;
                }
                else
                {
                    typename Types::Outcome outcome;
                    this->select(device, matrix_node->stats, outcome);

                    state.apply_actions(
                        state.row_actions[outcome.row_idx],
                        state.col_actions[outcome.col_idx]);
                    state.get_actions();

                    mutex_.lock();
                    ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                    MatrixNode *matrix_node_next = chance_node->access(state.obs);
                    mutex_.unlock();

                    MatrixNode *matrix_node_leaf = run_iteration(device, state, model, matrix_node_next, model_output);

                    outcome.value = model_output.value;
                    this->update_matrix_stats(matrix_node->stats, outcome, mutex);
                    this->update_chance_stats(chance_node->stats, outcome); // no guard
                    return matrix_node_leaf;
                }
            }
        };
    };
};
