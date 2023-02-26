#pragma once

#include "algorithm.hh"
#include "tree/node.hh"

template <typename Model>
class Exp3p : public Algorithm<Model>
{
public:
    struct MatrixStats : Algorithm<Model>::MatrixStats
    {
        int t = 0;
        typename Exp3p::VectorDouble row_gains = {0};
        typename Exp3p::VectorDouble col_gains = {0};
        typename Exp3p::VectorInt row_visits = {0};
        typename Exp3p::VectorInt col_visits = {0};

        int visits = 0;
        double row_value_total = 0;
        double col_value_total = 0;
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats
    {
        int visits = 0;
        double row_value_total = 0;
        double col_value_total = 0;
    };

    struct SearchParameters : Algorithm<Model>::SearchParameters
    {
        int playouts = 1;
    };

    Exp3p::SearchParameters search_parameters;

    prng &device;

    // Working memory
    typename Exp3p::VectorDouble row_forecast;
    typename Exp3p::VectorDouble col_forecast;

    Exp3p(prng &device) : device(device) {}

    void run () {}

    void run(
        int playouts,
        typename Exp3p::State &state,
        typename Exp3p::Model &model,
        MatrixNode<Exp3p> &root)
    {
        root.stats.t = playouts;
        for (int playout = 0; playout < playouts; ++playout)
        {
            typename Exp3p::State state_copy = state;
            this->playout(state_copy, model, &root);
        }
        // std::cout << "Exp3p root visits" << std::endl;
        // std::cout << "p0: " << root.stats.row_visits[0] << ' ' << root.stats.row_visits[1] << std::endl;
        // std::cout << "p1: " << root.stats.col_visits[0] << ' ' << root.stats.col_visits[1] << std::endl;
        // std::cout << "Exp3p root matrix" << std::endl;
        // get_matrix(&root).print();

        // typename Exp3p::VectorDouble strategy0;
        // typename Exp3p::VectorDouble strategy1;

        // std::cout << "strategies" << std::endl;
        // math::power_norm<Exp3p::VectorDouble>(root.stats.row_visits, root.legal_actions.rows, 1, strategy0);
        // for (int i = 0; i < root.legal_actions.rows; ++i)
        // {
        //     std::cout << strategy0[i] << ' ';
        // }
        // std::cout << std::endl;
        // math::power_norm<int, double, Exp3p::State::_size>(root.stats.col_visits, root.legal_actions.cols, 1, strategy1);
        // for (int j = 0; j < root.legal_actions.cols; ++j)
        // {
        //     std::cout << strategy1[j] << ' ';
        // }
        // std::cout << std::endl;
    }

    // Linear::Bimatrix2D<double, Exp3p::State::_size> get_matrix(MatrixNode<Exp3p> *matrix_node)
    // {
    //     Linear::Bimatrix2D<double, Exp3p::State::_size> M(matrix_node->legal_actions.rows, matrix_node->legal_actions.cols);
    //     for (int i = 0; i < M.rows; ++i)
    //     {
    //         for (int j = 0; j < M.cols; ++j)
    //         {
    //             M.set0(i, j, .5);
    //             M.set1(i, j, .5);
    //         }
    //     }

    //     // cur iterates through all chance node children
    //     ChanceNode<Exp3p> *cur = matrix_node->child;
    //     while (cur != nullptr)
    //     {
    //         M.set0(cur->row_idx, cur->col_idx, cur->stats.get_expected_value0());
    //         M.set1(cur->row_idx, cur->col_idx, cur->stats.get_expected_value1());
    //         cur = cur->next;
    //     }
    //     return M;
    // }

private:

    MatrixNode<Exp3p> *playout(
        typename Exp3p::State &state,
        Model &model,
        MatrixNode<Exp3p> *matrix_node)
    {
        /*
        Performs one playout of the growing tree algorithm.
        This recursive function returns the leaf node of the playout, which stores inference data for backpropogation
        */
        if (matrix_node->is_terminal == true)
        {
            return matrix_node;
        }
        else
        {
            if (matrix_node->is_expanded == true)
            {

                this->forecast(matrix_node);
                int row_idx = device.sample_pdf<typename Exp3p::VectorDouble>(this->row_forecast, matrix_node->legal_actions.rows);
                int col_idx = device.sample_pdf<typename Exp3p::VectorDouble>(this->col_forecast, matrix_node->legal_actions.cols);
                typename Exp3p::action_t action0 = matrix_node->legal_actions.actions0[row_idx];
                typename Exp3p::action_t action1 = matrix_node->legal_actions.actions1[col_idx];

                ChanceNode<Exp3p> *chance_node = matrix_node->access(row_idx, col_idx);
                MatrixNode<Exp3p> *matrix_node_next = chance_node->access(state.transition_data);
                MatrixNode<Exp3p> *matrix_node_leaf = this->playout(state, model, matrix_node_next);

                double row_payoff = matrix_node_leaf->inference.row_value;
                double col_payoff = matrix_node_leaf->inference.col_value;
                double row_inverse_prob = 1 / this->row_forecast[row_idx];
                double col_inverse_prob = 1 / this->col_forecast[col_idx];

                this->update_matrix_node(
                    matrix_node,
                    row_payoff, col_payoff,
                    row_idx, col_idx,
                    row_inverse_prob, col_inverse_prob);
                this->update_chance_node(chance_node, row_payoff, col_payoff);
                return matrix_node_leaf;
            }
            else
            {
                this->expand(state, model, matrix_node);
                return matrix_node;
            }
        }
    };

    inline void expand(
        typename Exp3p::State &state,
        Model model,
        MatrixNode<Exp3p> *matrix_node)
    {
        /*
        Expand a leaf node, right before we return it.
        */
        matrix_node->is_expanded = true;
        matrix_node->is_terminal = state.is_terminal;
        state.get_legal_actions(matrix_node->legal_actions);

        if (matrix_node->is_terminal)
        {
            matrix_node->inference.row_value = state.row_payoff;
            matrix_node->inference.col_value = state.col_payoff;
        }
        else
        {
            model.inference(state, matrix_node->legal_actions);
            matrix_node->inference_data = model.inference_data;
        }

        // Calculate Exp3p's time parameter using parent's.
        ChanceNode<Exp3p> *chance_parent = matrix_node->parent;
        if (chance_parent != nullptr)
        {
            MatrixNode<Exp3p> *matrix_parent = chance_parent->parent;
            int row_idx = parent->row_idx;
            int col_idx = parent->col_idx;
            double reach_probability = matrix_parent->inference.row_priors[row_idx] * matrix_parent->inference.col_priors[col_idx] * ((double)matrix_node->transition_data.probability);
            int t_estimate = matrix_parent->stats.t * reach_probability;
            t_estimate = t_estimate == 0 ? 1 : t_estimate;
            matrix_node->stats.t = t_estimate;
        }
    }

    inline void forecast(MatrixNode<Exp3p> *matrix_node)
    {
        /*
        Softmaxing of the gains to produce forecasts/strategies for the row and col players.
        The constants eta, gamma, beta are from (arXiv:1204.5721), Theorem 3.3.
        */
        const int time = matrix_node->stats.t;
        const int rows = matrix_node->legal_actions.rows;
        const int cols = matrix_node->legal_actions.cols;
        if (rows == 1)
        {
            this->row_forecast[0] = 1;
        }
        else
        {
            const double eta = .95 * sqrt(log(rows) / (time * rows));
            const double gamma_ = 1.05 * sqrt(rows * log(rows) / time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            this->softmax(this->row_forecast, matrix_node->stats.row_gains, rows, eta);
            for (int row_idx = 0; row_idx < rows; ++row_idx)
            {
                double x = (1 - gamma) * this->row_forecast[row_idx] + (gamma)*matrix_node->inference.row_priors[row_idx];
                this->row_forecast[row_idx] = x;
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
            this->softmax(this->col_forecast, matrix_node->stats.col_gains, cols, eta);
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                double x = (1 - gamma) * this->col_forecast[col_idx] + (gamma)*matrix_node->inference.col_priors[col_idx];
                this->col_forecast[col_idx] = x;
            }
        }
    }

    inline void softmax(typename Exp3p::VectorDouble &forecast, typename Exp3p::VectorDouble &gains, int k, double eta)
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

    inline void update_matrix_node(
        MatrixNode<Exp3p> *matrix_node,
        double row_payoff, double col_payoff,
        int row_idx, int col_idx,
        double row_inverse_prob, double col_inverse_prob)
    {
        matrix_node->stats.row_value_total += row_payoff;
        matrix_node->stats.col_value_total += col_payoff;
        matrix_node->stats.visits += 1;
        matrix_node->stats.row_visits[row_idx] += 1;
        matrix_node->stats.col_visits[col_idx] += 1;
        const double rows = matrix_node->pair_actions.rows;
        const double cols = matrix_node->pair_actions.cols;
        const double time = matrix_node->stats.time;
        const double row_beta = std::sqrt(std::log(rows) / (double)(time * rows));
        const double col_beta = std::sqrt(std::log(cols) / (double)(time * cols));
        matrix_node->stats.row_gains[row_idx] += row_beta * row_inverse_prob;
        matrix_node->stats.row_gains[row_idx] += col_beta * row_inverse_prob;
    }

    inline void update_chance_node(ChanceNode<Exp3p> *chance_node, double row_payoff, double col_payoff)
    {
        chance_node->stats.row_value_total += row_payoff;
        chance_node->stats.col_value_total += col_payoff;
        chance_node->stats.visits += 1;
    }
};