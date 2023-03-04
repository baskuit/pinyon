#pragma once

#include "../tree/node.hh"

#include <thread>
#include <mutex>

template <class _Model>
class AbstractAlgorithm
{
    // static_assert(std::derived_from<_Model, DualPolicyValueModel<typename _Model::Types::State>>,
    // "Even AbstractAlgorithm currently requires a DualPolicyValueModel");

public:
    struct MatrixStats
    {
    };
    struct ChanceStats
    {
    };
    struct Types : _Model::Types
    {
        using Model = _Model;
        using MatrixStats = AbstractAlgorithm::MatrixStats;
        using ChanceStats = AbstractAlgorithm::ChanceStats;
    };
};

template <class Model, class Algorithm>
class TreeBandit : public AbstractAlgorithm<Model>
{
public:
    struct Outcome;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using Outcome = TreeBandit::Outcome;
    };

    struct Outcome
    {
        int row_idx, col_idx;
        typename Types::Real row_value, col_value;
        typename Types::Real row_mu, col_mu; // Actor policy at row_idx, col_idx.
    };

    void run(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> &matrix_node)
    {
        init_stats(playouts, state, model, &matrix_node);
        for (int playout = 0; playout < playouts; ++playout)
        {
            this->playout(state, model, &matrix_node);
        }
    }

private:

    MatrixNode<Algorithm> *playout(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                Outcome outcome;

                this->select(matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = playout(state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                this->update_matrix_node(matrix_node, outcome);
                this->update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else
            {
                this->expand(state, model, matrix_node);
                return matrix_node;
            }
        }
        else
        {
            return matrix_node;
        }
    }

    void select(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        static_cast<Algorithm *>(this)->_select(
            matrix_node,
            outcome);
    }

    void init_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *root)
    {
        static_cast<Algorithm *>(this)->_init_stats(
            playouts,
            state,
            model,
            root);
    }
    void expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node)
    {
        static_cast<Algorithm *>(this)->_expand(
            state,
            model,
            matrix_node);
    }
    void update_matrix_node(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome)
    {
        static_cast<Algorithm *>(this)->_update_matrix_node(
            matrix_node,
            outcome);
    }

    void update_chance_node(
        ChanceNode<Algorithm> *chance_node,
        Outcome &outcome)
    {
        static_cast<Algorithm *>(this)->_update_chance_node(
            chance_node,
            outcome);
    }

};
/*
Node local bandit algorithm.
May not be possible to do this even with passing further derived class as template arg
simply because the type is still not complete when we get to the TreeBandit class
*/

// template <class Model, template <class _Model, class _BanditAlgorithm_> class _TreeBandit, class BanditDerived>
// class BanditAlgorithm : public TreeBandit<Model, BanditDerived>
// {
//     // static_assert(std::derived_from<Model, AbstractModel<typename Model::Types::State>>);
//     // We don't assume that Model is a DVPmodel.

// public:
//     struct Outcome; // If you don't add to types, you have to typename BanditAlgorithm<Model> in the specification...
//     struct Types : AbstractAlgorithm<Model>::Types
//     {
//         using Outcome = BanditAlgorithm::Outcome;
//     };
//     struct Outcome
//     {
//         int row_idx, col_idx;
//         typename Types::Real row_value, col_value;
//         typename Types::Real row_mu, col_mu; // Actor policy at row_idx, col_idx.
//     };
    // void select(
    //     MatrixNode<BanditAlgorithm> *matrix_node,
    //     Outcome &outcome);
    // void expand(
    //     typename Types::State &state,
    //     typename Types::Model &model,
    //     MatrixNode<BanditDerived> *matrix_node);
    // void _init_stats(
    //     typename Types::State &state,
    //     typename Types::Model &model,
    //     MatrixNode<BanditDerived> *matrix_node);
    // void update_matrix_node(
    //     MatrixNode<BanditDerived> *matrix_node,
    //     Outcome &outcome);
    // void update_chance_node(
    //     ChanceNode<BanditDerived> *chance_node,
    //     Outcome &outcome);
// };

/*
Exp3p
*/

template <class Model, template <class _Model, class _BanditAlgorithm_> class _TreeBandit>
class Exp3p : public _TreeBandit<Model, Exp3p<Model, _TreeBandit>>
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::Types
    {
        using MatrixStats = Exp3p::MatrixStats;
        using ChanceStats = Exp3p::ChanceStats;
    };
    struct MatrixStats : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::MatrixStats
    {
        int time = 0;
        typename Types::VectorReal row_gains = {0};
        typename Types::VectorReal col_gains = {0};
        typename Types::VectorInt row_visits = {0};
        typename Types::VectorInt col_visits = {0};

        int visits = 0;
        double row_value_total = 0;
        double col_value_total = 0;
    };

    struct ChanceStats : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::ChanceStats
    {
        int visits = 0;
        double row_value_total = 0;
        double col_value_total = 0;
    };

    prng &device;
    typename Types::VectorReal row_forecast;
    typename Types::VectorReal col_forecast;

    Exp3p(prng &device) : device(device) {}

    void _init_stats(
        int playouts,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Exp3p> *matrix_node)
    {
        matrix_node->stats.time = playouts;
    }

    void _expand(
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Exp3p> *matrix_node)
    {
        /*
        Expand a leaf node, right before we return it.
        */
        matrix_node->is_expanded = true;
        state.get_actions();
        matrix_node->is_terminal = state.is_terminal;
        matrix_node->actions = state.actions;

        if (matrix_node->is_terminal)
        {
            matrix_node->inference.row_value = state.row_payoff;
            matrix_node->inference.col_value = state.col_payoff;
        }
        else
        {
            model.get_inference(state, matrix_node->inference);
        }

        // Calculate Exp3p's time parameter using parent's.
        ChanceNode<Exp3p> *chance_parent = matrix_node->parent;
        if (chance_parent != nullptr)
        {
            MatrixNode<Exp3p> *matrix_parent = chance_parent->parent;
            int row_idx = chance_parent->row_idx;
            int col_idx = chance_parent->col_idx;
            double reach_probability =
                matrix_parent->inference.row_policy[row_idx] *
                matrix_parent->inference.col_policy[col_idx] *
                ((double)matrix_node->transition.prob);
            int time_estimate = matrix_parent->stats.time * reach_probability;
            time_estimate = time_estimate == 0 ? 1 : time_estimate;
            matrix_node->stats.time = time_estimate;
        }
    }

    void _select(
        MatrixNode<Exp3p> *matrix_node,
        typename Types::Outcome &outcome)
    {
        /*
        Softmaxing of the gains to produce forecasts/strategies for the row and col players.
        The constants eta, gamma, beta are from (arXiv:1204.5721), Theorem 3.3.
        */
        const int time = matrix_node->stats.time;
        const int rows = matrix_node->actions.rows;
        const int cols = matrix_node->actions.cols;
        if (rows == 1)
        {
            this->row_forecast[0] = 1;
        }
        else
        {
            const double eta = .95 * sqrt(log(rows) / (time * rows));
            const double gamma_ = 1.05 * sqrt(rows * log(rows) / time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            softmax(this->row_forecast, matrix_node->stats.row_gains, rows, eta);
            for (int row_idx = 0; row_idx < rows; ++row_idx)
            {
                this->row_forecast[row_idx] =
                    (1 - gamma) * this->row_forecast[row_idx] +
                    (gamma)*matrix_node->inference.row_policy[row_idx];
            }
        }
        if (cols == 1)
        {
            this->col_forecast[0] = 1;
        }
        else
        {
            const double eta = .95 * sqrt(log(cols) / (time * cols));
            const double gamma_ = 1.05 * sqrt(cols * log(cols) / time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            softmax(this->col_forecast, matrix_node->stats.col_gains, cols, eta);
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                this->col_forecast[col_idx] =
                    (1 - gamma) * this->col_forecast[col_idx] +
                    (gamma)*matrix_node->inference.col_policy[col_idx];
            }
        }
        const int row_idx = this->device.sample_pdf(row_forecast, rows);
        const int col_idx = this->device.sample_pdf(col_forecast, cols); 
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
        outcome.row_mu = row_forecast[row_idx];
        outcome.col_mu = col_forecast[col_idx];
    }

    void _update_matrix_node(
        MatrixNode<Exp3p> *matrix_node, 
        typename Types::Outcome &outcome)
    {
        matrix_node->stats.row_value_total += outcome.row_value;
        matrix_node->stats.col_value_total += outcome.col_value;
        matrix_node->stats.visits += 1;
        matrix_node->stats.row_visits[outcome.row_idx] += 1;
        matrix_node->stats.col_visits[outcome.col_idx] += 1;
        const double rows = matrix_node->actions.rows;
        const double cols = matrix_node->actions.cols;
        const double time = matrix_node->stats.time;
        const double row_beta = std::sqrt(std::log(rows) / (double)(time * rows));
        const double col_beta = std::sqrt(std::log(cols) / (double)(time * cols));
        matrix_node->stats.row_gains[outcome.row_idx] += outcome.row_value / outcome.row_mu + row_beta; // TODO check this lmao
        matrix_node->stats.col_gains[outcome.col_idx] += outcome.col_value / outcome.col_mu + col_beta;
    }

    void _update_chance_node(
        ChanceNode<Exp3p> *chance_node,
        typename Types::Outcome &outcome)
    {
        chance_node->stats.row_value_total += outcome.row_value;
        chance_node->stats.col_value_total += outcome.col_value;
        chance_node->stats.visits += 1;
    }

private:

    inline void softmax(
        typename Types::VectorReal &forecast,
        typename Types::VectorReal &gains,
        int k,
        typename Types::Real eta)
    {
        /*
        Softmax helper function with logit scaling and uniform noise.
        */
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
};