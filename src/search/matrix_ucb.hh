#pragma once

#include "algorithm.hh"
#include "tree/node.hh"

template <typename Model> 
class MatrixUCB : public Algorithm<Model> {
public:

    struct MatrixStats : Algorithm<Model>::MatrixStats {
        Linear::Bimatrix2D<double, Model::state_t::size_> cumulative_payoffs;
        Linear::Matrix2D<int, Model::state_t::size_> visits;
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats {
        // Don't need stats here since we store them in MatrixStats

        // double value0 () {return visits > 0 ? cumulative_value0 / visits : .5;}
        // double value1 () {return visits > 0 ? cumulative_value1 / visits : .5;}
    };

    prng& device;

    MatrixUCB (prng& device)  :
        device(device) {}

    void expand (
        typename MatrixUCB::state_t& state, 
        Model model, 
        MatrixNode<MatrixUCB>* matrix_node
    ) {
        matrix_node->expanded = true;
        state.actions(matrix_node->pair);
        matrix_node->terminal = (matrix_node->pair.rows * matrix_node->pair.cols == 0);

        if (matrix_node->terminal) { // Makes this model independent 
            matrix_node->inference.value_estimate0 = state.payoff0;
            matrix_node->inference.value_estimate1 = state.payoff1;
        } else {
            model.inference(state, matrix_node->pair);
            matrix_node->inference = model.inference_; // Inference expects a state, gets T instead...
        }

        matrix_node->stats.cumulative_payoffs.rows = matrix_node->pair.rows;
        matrix_node->stats.cumulative_payoffs.cols = matrix_node->pair.cols;
        matrix_node->stats.visits.rows = matrix_node->pair.rows;
        matrix_node->stats.visits.cols = matrix_node->pair.cols;
        for (int row_idx = 0; row_idx < matrix_node->pair.rows; ++row_idx) {
            for (int col_idx = 0; col_idx < matrix_node->pair.cols; ++col_idx) {
                matrix_node->stats.cumulative_payoffs.set0(row_idx, col_idx, 0);
                matrix_node->stats.cumulative_payoffs.set1(row_idx, col_idx, 0);
                matrix_node->stats.visits.set(row_idx, col_idx, 0);
            }
        }
    }

    MatrixNode<MatrixUCB>* runPlayout ( 
        typename MatrixUCB::state_t& state, 
        typename MatrixUCB::model_t& model, 
        MatrixNode<MatrixUCB>* matrix_node
    ) {

        if (matrix_node->terminal == true) {
        std::cout << "Termanal" << std::endl;
            return matrix_node;
        } else {
            std::array<double, 9> strategy0 = {0};
            std::array<double, 9> strategy1 = {0};
            Linear::Bimatrix2D<double, MatrixUCB::state_t::size_> A(matrix_node->pair.rows, matrix_node->pair.cols);
            
            process_matrix(matrix_node->stats.cumulative_payoffs,matrix_node->stats.visits,A);




            if (matrix_node->expanded == true) {

                Bandit::SolveBimatrix<double, MatrixUCB::state_t::size_> (
                    device,
                    1000,
                    A,
                    strategy0,
                    strategy1
                );
            std::cout << "Strategies" << std::endl;
            std::cout << strategy0[0] << ' ' << strategy0[1] << std::endl;
            std::cout << strategy0[0] << ' ' << strategy0[1] << std::endl;
                int row_idx = device.sample_pdf<double, MatrixUCB::state_t::size_>(strategy0, matrix_node->pair.rows);
                int col_idx = device.sample_pdf<double, MatrixUCB::state_t::size_>(strategy1, matrix_node->pair.cols);

                typename MatrixUCB::action_t action0 = matrix_node->pair.actions0[row_idx];
                typename MatrixUCB::action_t action1 = matrix_node->pair.actions1[col_idx];
                typename MatrixUCB::transition_data_t transition_data = state.transition(action0, action1);

                ChanceNode<MatrixUCB>* chance_node = matrix_node->access(row_idx, col_idx);
                MatrixNode<MatrixUCB>* matrix_node_next = chance_node->access(transition_data);
                    MatrixNode<MatrixUCB>* matrix_node_leaf = runPlayout(state, model, matrix_node_next);

                double u0 = matrix_node_leaf->inference.value_estimate0;
                double u1 = matrix_node_leaf->inference.value_estimate1;
                update(matrix_node, u0, u1, row_idx, col_idx);                return matrix_node_leaf;
            } else {
                expand(state, model, matrix_node);
            std::cout << "Termanal" << std::endl;
                return matrix_node;
            }
        }
    };

    void search (
        int playouts,
        typename MatrixUCB::state_t& state, 
        MatrixNode<MatrixUCB>* root
    ) {
        typename MatrixUCB::model_t model(device);
        
        for (int playout = 0; playout < playouts; ++playout) {
            auto state_ = state;
            runPlayout(state_, model, root);
        }
    }

private:

    void update (MatrixNode<MatrixUCB>* matrix_node, double u0, double u1, int row_idx, int col_idx) {
        const double v0 = matrix_node->stats.cumulative_payoffs.get0(row_idx, col_idx);
        const double v1 = matrix_node->stats.cumulative_payoffs.get1(row_idx, col_idx);
        const int n = matrix_node->stats.visits.get(row_idx, col_idx);
        matrix_node->stats.cumulative_payoffs.set0(row_idx, col_idx, v0 + u0);
        matrix_node->stats.cumulative_payoffs.set1(row_idx, col_idx, v1 + u1);
        matrix_node->stats.visits.set(row_idx, col_idx, n+1);
        // TODO add increment overloads
    }

    void process_matrix (
        Linear::Bimatrix<double, MatrixUCB::state_t::size_>& cumulative_payoffs, 
        Linear::Matrix<int, MatrixUCB::state_t::size_>& visits, 
        Linear::Bimatrix<double, MatrixUCB::state_t::size_>& output
    ) {
        // assert dimensions make sense TODO
        // rename TODO
        const int rows = output.rows;
        const int cols = output.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx) {
            for (int col_idx = 0; col_idx < cols; ++col_idx) {
                double u = cumulative_payoffs.get0(row_idx, col_idx);
                double v = cumulative_payoffs.get1(row_idx, col_idx);
                int n = visits.get(row_idx, col_idx);
                n += (n == 0);
                double a = n > 0 ? u / n : .5;
                double b = n > 0 ? v / n : .5;
                int t = 100; //todo
                const double x = a + std::sqrt(2 * std::log(2 * t * t * rows * cols) / n);
                const double y = b + std::sqrt(2 * std::log(2 * t * t * rows * cols) / n);
                output.set0(row_idx, col_idx, x);
                output.set1(row_idx, col_idx, y);
            }
        }
    }
};