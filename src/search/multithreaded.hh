#pragma once

#include "algorithm.hh"
#include "tree/node.hh"

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

    MatrixNode<Algorithm> *_playout(
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
                this->select(matrix_node, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->_playout(state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                mtx.lock();
                this->update_matrix_node(matrix_node, outcome);
                this->update_chance_node(chance_node, outcome);
                mtx.unlock();
                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                this->expand(state, model, matrix_node);
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
    std::atomic<int> current_index{};
    // we simply let this overflow or w/e
    // TODO test overflow behaviour

    MatrixNode<Algorithm> *_playout(
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
                this->select(matrix_node, outcome);
                mtx.unlock();

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->_playout(state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                mtx.lock();
                this->update_matrix_node(matrix_node, outcome);
                this->update_chance_node(chance_node, outcome);
                mtx.unlock();
                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                this->expand(state, model, matrix_node);
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

/*
TODO: Refer to old code for multithreading
*/

// void loopPlayout(
//     int playouts,
//     typename Exp3p::state_t *state,
//     MatrixNode<Exp3p> *matrix_node)
// {
//     prng device_(device.random_int(2147483647));
//     typename Exp3p::model_t model(device_);

//     for (int playout = 0; playout < playouts; ++playout)
//     {
//         auto state_ = *state;
//         runPlayout(state_, model, matrix_node);
//     }
// }

// // Top level search function
// void search(
//     int threads,
//     int playouts,
//     typename Exp3p::state_t &state,
//     MatrixNode<Exp3p> *root)
// {
//     root->stats.t = playouts;
//     std::thread thread_pool[threads];

//     for (int i = 0; i < threads; ++i)
//     {
//         const int playouts_thread = playouts / threads;
//         thread_pool[i] = std::thread(&Exp3p::loopPlayout, this, playouts_thread, &state, root);
//     }
//     for (int i = 0; i < threads; ++i)
//     {
//         thread_pool[i].join();
//     }
// }