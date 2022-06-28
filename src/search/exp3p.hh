#pragma once

#include "algorithm.hh"
#include "tree/node.hh"

#include <chrono>
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

template <typename Model> 
class Exp3p : public Algorithm<Model> {
public:

    struct MatrixStats : Algorithm<Model>::MatrixStats {
        int t = 0;
        std::array<double, Exp3p::state_t::size_> gains0 = {0};
        std::array<double, Exp3p::state_t::size_> gains1 = {0};
        std::array<int, Exp3p::state_t::size_> visits0 = {0};
        std::array<int, Exp3p::state_t::size_> visits1 = {0};

        int visits = 0;
        double cumulative_value0 = 0;
        double cumulative_value1 = 0;
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats {
        int visits = 0;
        double cumulative_value0 = 0;
        double cumulative_value1 = 0;
    };

    prng& device;

    Exp3p (prng& device)  :
        device(device) {}

    // Working memory
    std::array<double, Exp3p::state_t::size_> forecast0;
    std::array<double, Exp3p::state_t::size_> forecast1;

    void expand (typename Exp3p::state_t& state, Model model, MatrixNode<Exp3p>* matrix_node) {
        matrix_node->expanded = true;
        state.actions(matrix_node->pair);
        matrix_node->terminal = (matrix_node->pair.rows * matrix_node->pair.cols == 0);// (matrix_node->pair.rows*matrix_node->pair.cols == 0);

        if (matrix_node->terminal) { // Makes this model independent 
            matrix_node->inference.value_estimate0 = state.payoff0;
            matrix_node->inference.value_estimate1 = state.payoff1;
        } else {
            model.inference(state, matrix_node->pair);
            
            matrix_node->inference = model.inference_; // Inference expects a state, gets T instead...
        }
        if (matrix_node->parent != nullptr) {
            int t_estimate = matrix_node->parent->parent->stats.t / 4;
            t_estimate = t_estimate == 0 ? 1 : t_estimate;
            matrix_node->stats.t = t_estimate;
        }
    }

    MatrixNode<Exp3p>* search (typename Exp3p::state_t& state, typename Exp3p::model_t model, MatrixNode<Exp3p>* matrix_node) {

        if (matrix_node->terminal == true) {
            return matrix_node;
        } else {
            if (matrix_node->expanded == true) {
                forecast(matrix_node);
                int row_idx = device.sample_pdf<double, Exp3p::state_t::size_>(forecast0, matrix_node->pair.rows);
                int col_idx = device.sample_pdf<double, Exp3p::state_t::size_>(forecast1, matrix_node->pair.cols);
                double inverse_prob0 = 1/forecast0[row_idx];
                double inverse_prob1 = 1/forecast1[col_idx]; // Forecast is altered by subsequent search calls

                typename Exp3p::action_t action0 = matrix_node->pair.actions0[row_idx];
                typename Exp3p::action_t action1 = matrix_node->pair.actions1[col_idx];
                typename Exp3p::transition_data_t transition_data = state.transition(action0, action1);

                ChanceNode<Exp3p>* chance_node = matrix_node->access(row_idx, col_idx);
                MatrixNode<Exp3p>* matrix_node_next = chance_node->access(transition_data);
                    MatrixNode<Exp3p>* matrix_node_leaf = search(state, model, matrix_node_next);

                double u0 = matrix_node_leaf->inference.value_estimate0;
                double u1 = matrix_node_leaf->inference.value_estimate1;
                matrix_node->stats.gains0[row_idx] += u0 * inverse_prob0;
                matrix_node->stats.gains1[col_idx] += u1 * inverse_prob1;
                update(matrix_node, u0, u1, row_idx, col_idx);
                update(chance_node, u0, u1);
                return matrix_node_leaf;
            } else {
                expand(state, model, matrix_node);
                return matrix_node;
            }
        }
    };

private:

    // Softmax and uniform noise
    void softmax (std::array<double, Exp3p::state_t::size_>& forecast, std::array<double, Exp3p::state_t::size_>& gains, int k, double eta) {
        double max = 0;
        for (int i = 0; i < k; ++i) {
            double x = gains[i];
            if (x > max) {
                max = x;
            } 
        }
        double sum = 0;
        for (int i = 0; i < k; ++i) {
            gains[i] -= max;
            double x = gains[i];
            double y = std::exp(x * eta);
            forecast[i] = y;
            sum += y;
        }
        for (int i = 0; i < k; ++i) {
            forecast[i] /= sum;
        }
    };

    void forecast (MatrixNode<Exp3p>* matrix_node) {
        const int time = matrix_node->stats.t;
        const int rows = matrix_node->pair.rows;
        const int cols = matrix_node->pair.cols;
        if (rows == 1) {
            forecast0[0] = 1;
        } else {
            const double eta = .95 * sqrt(log(rows)/(time*rows));
            const double gamma_ = 1.05 * sqrt(rows*log(rows)/time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            softmax(forecast0, matrix_node->stats.gains0, rows, eta);
            for (int row_idx = 0; row_idx < rows; ++row_idx) {
                double x = (1 - gamma) * forecast0[row_idx] + (gamma) * matrix_node->inference.strategy_prior0[row_idx];
                forecast0[row_idx] = x;
         
            }
        }
        if (cols == 1) {
            forecast1[0] = 1;
        } else {
            const double eta = .95 * sqrt(log(cols)/(time*cols));
            const double gamma_ = 1.05 * sqrt(cols*log(cols)/time);
            const double gamma = gamma_ < 1 ? gamma_ : 1;
            softmax(forecast1, matrix_node->stats.gains1, cols, eta);
            for (int col_idx = 0; col_idx < cols; ++col_idx) {
                double x = (1 - gamma) * forecast1[col_idx] + (gamma) * matrix_node->inference.strategy_prior1[col_idx];
                forecast1[col_idx] = x;
            }
        }
    }

    void update (MatrixNode<Exp3p>* matrix_node, double u0, double u1, int row_idx, int col_idx) {
        matrix_node->stats.cumulative_value0 += u0;
        matrix_node->stats.cumulative_value1 += u1;
        matrix_node->stats.visits += 1; 
        matrix_node->stats.visits0[row_idx] += 1;
        matrix_node->stats.visits1[col_idx] += 1;
    }
    void update (ChanceNode<Exp3p>* chance_node, double u0, double u1) {
        chance_node->stats.cumulative_value0 += u0;
        chance_node->stats.cumulative_value1 += u1;
        chance_node->stats.visits += 1;
    }
};