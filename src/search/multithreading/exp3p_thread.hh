#pragma once

#include "../algorithm.hh"
#include "tree/node.hh"

#include <thread>
#include <mutex>

/*
Last attempted change was to make model a parameter of search but
I'm not sure if that's thread safe.

Currently it avoids this by using differnt model for each thread?
*/

template <typename Model>
class Exp3p : public Algorithm<Model>
{
public:
    struct MatrixStats : Algorithm<Model>::MatrixStats
    {
        int t = 0; // expected number of playouts
        std::array<double, Exp3p::state_t::_size> gains0 = {0};
        std::array<double, Exp3p::state_t::_size> gains1 = {0};
        std::array<int, Exp3p::state_t::_size> visits0 = {0};
        std::array<int, Exp3p::state_t::_size> visits1 = {0};

        int visits = 0;
        double cumulative_value0 = 0;
        double cumulative_value1 = 0;

        std::mutex mtx;
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats
    {
        int visits = 0;
        double cumulative_value0 = 0;
        double cumulative_value1 = 0;
        double get_expected_value0() { return visits > 0 ? cumulative_value0 / visits : .5; }
        double get_expected_value1() { return visits > 0 ? cumulative_value1 / visits : .5; }
    };

    prng device; // used to instantiate thread local prng

    Exp3p(prng &device) : device(device) {}

    void expand(
        typename Exp3p::state_t &state,
        Model model,
        MatrixNode<Exp3p> *matrix_node)
    {

        matrix_node->is_expanded = true;

        // actions
        state.get_legal_actions(matrix_node->legal_actions);
        matrix_node->is_terminal = (matrix_node->legal_actions.rows * matrix_node->legal_actions.cols == 0);

        // inference
        if (matrix_node->is_terminal)
        {
            matrix_node->inference.value0 = state.payoff0;
            matrix_node->inference.value1 = state.payoff1;
        }
        else
        {
            model.inference(state, matrix_node->legal_actions);
            matrix_node->inference = model.last_inference; // One model per thread
        }

        // time and mutex
        ChanceNode<Exp3p> *parent = matrix_node->parent;
        if (parent != nullptr)
        {
            MatrixNode<Exp3p> *mparent = parent->parent;
            int row_idx = parent->row_idx;
            int col_idx = parent->col_idx;
            double joint_p = mparent->inference.strategy_prior0[row_idx] * mparent->inference.strategy_prior1[col_idx] * ((double)matrix_node->transition_data.probability);
            int t_estimate = matrix_node->parent->parent->stats.t * joint_p;
            t_estimate = t_estimate == 0 ? 1 : t_estimate;
            matrix_node->stats.t = t_estimate;
        }
    }

    MatrixNode<Exp3p> *runPlayout(
        typename Exp3p::state_t &state,
        typename Exp3p::model_t &model,
        MatrixNode<Exp3p> *matrix_node)
    {

        std::mutex &mtx = matrix_node->stats.mtx;

        if (matrix_node->is_terminal == true)
        {
            return matrix_node;
        }
        else
        {
            if (matrix_node->is_expanded == true)
            {
                std::array<double, Exp3p::state_t::_size> forecast0;
                std::array<double, Exp3p::state_t::_size> forecast1;
                mtx.lock();
                forecast(matrix_node, forecast0, forecast1);
                mtx.unlock();
                int row_idx = device.sample_pdf<double, Exp3p::state_t::_size>(forecast0, matrix_node->legal_actions.rows);
                int col_idx = device.sample_pdf<double, Exp3p::state_t::_size>(forecast1, matrix_node->legal_actions.cols);

                typename Exp3p::action_t action0 = matrix_node->legal_actions.actions0[row_idx];
                typename Exp3p::action_t action1 = matrix_node->legal_actions.actions1[col_idx];
                typename Exp3p::transition_data_t transition_data = state.apply_actions(action0, action1);

                ChanceNode<Exp3p> *chance_node = matrix_node->access(row_idx, col_idx);
                MatrixNode<Exp3p> *matrix_node_next = chance_node->access(transition_data);

                MatrixNode<Exp3p> *matrix_node_leaf = runPlayout(state, model, matrix_node_next);

                double u0 = matrix_node_leaf->inference.value0;
                double u1 = matrix_node_leaf->inference.value1;
                mtx.lock();
                matrix_node->stats.gains0[row_idx] += u0 / forecast0[row_idx];
                matrix_node->stats.gains1[col_idx] += u1 / forecast1[col_idx];
                update(matrix_node, u0, u1, row_idx, col_idx);
                update(chance_node, u0, u1);
                mtx.unlock();
                return matrix_node_leaf;
            }
            else
            {
                mtx.lock();
                expand(state, model, matrix_node);
                mtx.unlock();
                return matrix_node;
            }
        }
    };

    // Called by each thread. Pass state by value and instantiate a new model to guarantee no shared resources
    // PbV state will hard copy the prng, so each thread has same but separate state prng.
    // Toy state does not use this anyway
    void loopPlayout(
        int playouts,
        typename Exp3p::state_t *state,
        typename Exp3p::model_t *model,
        MatrixNode<Exp3p> *matrix_node)
    {
        prng device_(device.random_int(2147483647));
        typename Exp3p::model_t model_ = *model;
        model_.device = device_;

        for (int playout = 0; playout < playouts; ++playout)
        {
            auto state_ = *state;
            runPlayout(state_, model_, matrix_node);
        }
    }

    // Top level search function
    void search(
        int threads,
        int playouts,
        typename Exp3p::state_t &state,
        typename Exp3p::model_t &model,
        MatrixNode<Exp3p> &root)
    {
        root.stats.t = playouts;
        std::thread thread_pool[threads];

        for (int i = 0; i < threads; ++i)
        {
            const int playouts_thread = playouts / threads;
            thread_pool[i] = std::thread(&Exp3p::loopPlayout, this, playouts_thread, &state, &model, &root);
        }

        for (int i = 0; i < threads; ++i)
        {
            thread_pool[i].join();
        }
    }

    Linear::Bimatrix2D<double, Exp3p::state_t::_size> get_matrix(MatrixNode<Exp3p> *matrix_node)
    {
        Linear::Bimatrix2D<double, Exp3p::state_t::_size> M(matrix_node->legal_actions.rows, matrix_node->legal_actions.cols);
        for (int i = 0; i < M.rows; ++i)
        {
            for (int j = 0; j < M.cols; ++j)
            {
                M.set0(i, j, .5);
                M.set1(i, j, .5);
            }
        }

        // cur iterates through all chance node children
        ChanceNode<Exp3p> *cur = matrix_node->child;
        std::cout << "iterating through children" << std::endl;
        while (cur != nullptr)
        {
            M.set0(cur->row_idx, cur->col_idx, cur->stats.get_expected_value0());
            M.set1(cur->row_idx, cur->col_idx, cur->stats.get_expected_value1());
            std::cout << cur->row_idx << ' ' << cur->col_idx << ' ' << cur->stats.get_expected_value0() << std::endl;
            cur = cur->next;
        }
        return M;
    }


private:
    // Softmax and uniform noise
    void softmax(
        std::array<double, Exp3p::state_t::_size> &forecast,
        std::array<double, Exp3p::state_t::_size> &gains,
        int k,
        double eta)
    {
        double max = 0;
        for (int i = 0; i < k; ++i)
        {
            double x = gains[i];
            if (x > max)
            {
                max = x;
            }
        }
        double sum = 0;
        for (int i = 0; i < k; ++i)
        {
            gains[i] -= max;
            double x = gains[i];
            double y = std::exp(x * eta);
            forecast[i] = y;
            sum += y;
        }
        for (int i = 0; i < k; ++i)
        {
            forecast[i] /= sum;
        }
    };

    void forecast(
        MatrixNode<Exp3p> *matrix_node,
        std::array<double, Exp3p::state_t::_size> &forecast0,
        std::array<double, Exp3p::state_t::_size> &forecast1)
    {
        const int time = matrix_node->stats.t;
        const int rows = matrix_node->legal_actions.rows;
        const int cols = matrix_node->legal_actions.cols;
        if (rows == 1)
        {
            forecast0[0] = 1;
        }
        else
        {
            const double eta = .95 * sqrt(log(rows) / (time * rows));
            const double gamma_ = 1.05 * sqrt(rows * log(rows) / time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            softmax(forecast0, matrix_node->stats.gains0, rows, eta);
            for (int row_idx = 0; row_idx < rows; ++row_idx)
            {
                double x = (1 - gamma) * forecast0[row_idx] + (gamma)*matrix_node->inference.strategy_prior0[row_idx];
                forecast0[row_idx] = x;
            }
        }
        if (cols == 1)
        {
            forecast1[0] = 1;
        }
        else
        {
            const double eta = .95 * sqrt(log(cols) / (time * cols));
            const double gamma_ = 1.05 * sqrt(cols * log(cols) / time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            softmax(forecast1, matrix_node->stats.gains1, cols, eta);
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                double x = (1 - gamma) * forecast1[col_idx] + (gamma)*matrix_node->inference.strategy_prior1[col_idx];
                forecast1[col_idx] = x;
            }
        }
    }

    void update(
        MatrixNode<Exp3p> *matrix_node,
        double u0,
        double u1,
        int row_idx,
        int col_idx)
    {

        matrix_node->stats.cumulative_value0 += u0;
        matrix_node->stats.cumulative_value1 += u1;
        matrix_node->stats.visits += 1;
        matrix_node->stats.visits0[row_idx] += 1;
        matrix_node->stats.visits1[col_idx] += 1;
    }
    void update(ChanceNode<Exp3p> *chance_node, double u0, double u1)
    {
        chance_node->stats.cumulative_value0 += u0;
        chance_node->stats.cumulative_value1 += u1;
        chance_node->stats.visits += 1;
    }
};