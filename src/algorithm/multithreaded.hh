#pragma once

#include "algorithm.hh"
#include "tree/tree.hh"

#include <thread>
#include <mutex>
#include <atomic>

/*
This module needs a lot of work for general use.
The function for multithreaded search clones the model since I don't know how to make those thread safe...

I could make a new class of models that are marked as thread-safe. Libtorch stuff and maybe a example of MonteCarlo

*/


/*
TreeBandit with a mutex in each matrix node
*/

template <class Model, class Algorithm>
class TreeBanditThreaded : public TreeBanditBase<Model, Algorithm>
{
public:
    struct MatrixStats;
    struct Types : TreeBanditBase<Model, Algorithm>::Types
    {
        using MatrixStats = TreeBanditThreaded::MatrixStats;
    };

    struct MatrixStats
    {
        std::mutex mtx;
    };

    int threads = 1;

    // Override the TreeBanditBase run for threads
    void run(
        int playouts,
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> &matrix_node)
    {
        this->_initialize_stats(playouts, state, model, &matrix_node);
        std::thread thread_pool[threads];
        const int playouts_per_thread = playouts / threads;
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i] = std::thread(&TreeBanditThreaded::runThread, this, playouts_per_thread, &device, &state, &model, &matrix_node);
        }
        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i].join();
        }
    }

    void runThread(
        int playouts,
        prng *device,
        typename Types::State *state,
        typename Types::Model *model,
        MatrixNode<Algorithm> *matrix_node)
    {
        prng device_thread; // TODO deterministically provide new seed
        typename Types::Model model_thread = *model;
        for (int playout = 0; playout < playouts; ++playout)
        {
            typename Types::State state_copy = *state;
            this->_playout(*device, state_copy, *model, matrix_node);
        }
    }

    MatrixNode<Algorithm> *playout(
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
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

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->playout(device, state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                mtx.lock();
                this->_update_matrix_node(matrix_node, outcome);
                this->_update_chance_node(chance_node, outcome);
                mtx.unlock();
                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                this->_expand(state, model, matrix_node);
                mtx.unlock();
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }
};

/*

Currently doesn't even compile because of the extra pool_size template parameter... TODO TODO TODO

*/

/*
TreeBandit with a mutex pool
*/

template <class Model, class Algorithm, int pool_size>
class TreeBanditThreadPool : public TreeBanditBase<Model, Algorithm>
{
    // static_assert(std::derived_from<TreeBanditThreadPool<Model, Algorithm, pool_size>, Algorithm>,
    // "ThreadBanditThreadPool should be a derived class of its template Algorithm (Bandit) arg");
public:
    struct MatrixStats;
    struct Types : TreeBanditBase<Model, Algorithm>::Types
    {
        using MatrixStats = TreeBanditThreadPool::MatrixStats;
    };

    struct MatrixStats
    {
        int mutex_index = 0;
    };

    static const std::array<std::mutex, pool_size> mutex_pool;
    std::atomic<int> current_index{0};
    // we simply let this overflow or w/e
    // TODO test overflow behaviour

    // void run(
    //     int playouts,
    //     prng &device,
    //     typename Types::State &state,
    //     typename Types::Model &model,
    //     MatrixNode<Algorithm> &matrix_node)
    // {
    //     this->_initialize_stats(playouts, state, model, &matrix_node);
    //     std::thread thread_pool[threads];
    //     const int playouts_per_thread = playouts / threads;
    //     for (int i = 0; i < threads; ++i)
    //     {
    //         thread_pool[i] = std::thread(&TreeBanditThreaded::runThread, this, playouts_per_thread, &device, &state, &model, &matrix_node);
    //     }
    //     for (int i = 0; i < threads; ++i)
    //     {
    //         thread_pool[i].join();
    //     }
    // }

    // void runThread(
    //     int playouts,
    //     prng *device,
    //     typename Types::State *state,
    //     typename Types::Model *model,
    //     MatrixNode<Algorithm> *matrix_node)
    // {
    //     prng device_thread(device->get_seed());
    //     typename Types::Model model_thread = *model;
    //     for (int playout = 0; playout < playouts; ++playout)
    //     {
    //         typename Types::State state_copy = *state;
    //         this->_playout(*device, state_copy, *model, matrix_node);
    //     }
    // }

    MatrixNode<Algorithm> *playout(
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {

        std::mutex &mtx = mutex_pool[matrix_node->stats.mutex_idx];

        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                typename Types::Outcome outcome;

                mtx.lock();
                this->_select(device, matrix_node, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->playout(device, state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;

                mtx.lock();
                this->_update_matrix_node(matrix_node, outcome);
                this->_update_chance_node(chance_node, outcome);
                mtx.unlock();

                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                this->_expand(state, model, matrix_node);
                this->get_mutex_index(matrix_node);
                mtx.unlock();

                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }

private:
    void get_mutex_index(
        MatrixNode<Algorithm> *matrix_node)
    {
        matrix_node->stats.mutex_index = (this->current_index++) % pool_size;
    }
};