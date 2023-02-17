#pragma once

#include "algorithm.hh"
#include "tree/node.hh"

#include <thread>
#include <mutex>

template <typename Model, int pool_size> 
class MatrixUCB : public Algorithm<Model> {
public:

    struct MatrixStats : Algorithm<Model>::MatrixStats {
        int t = 0;
        Linear::Bimatrix2D<double, Model::state_t::size_> cumulative_payoffs;
        Linear::Matrix2D<int, Model::state_t::size_> visits;
        std::array<double, Model::state_t::size_> strategy0;
        std::array<double, Model::state_t::size_> strategy1;  
        int mutex_idx = 0;   
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats {
        // Don't need stats here since we store them in MatrixStats

        // double value0 () {return visits > 0 ? cumulative_value0 / visits : .5;}
        // double value1 () {return visits > 0 ? cumulative_value1 / visits : .5;}
    };

    prng& device;
    std::array<std::mutex, pool_size> mutex_pool;

    MatrixUCB (prng& device)  :
        device(device) {}

    void expand (
        typename MatrixUCB::state_t& state, 
        Model model, 
        MatrixNode<MatrixUCB>* matrix_node
    ) {
        matrix_node->is_expanded = true;
        state.get_legal_actions(matrix_node->legal_actions);
        matrix_node->is_terminal = (matrix_node->legal_actions.rows * matrix_node->legal_actions.cols == 0);

        if (matrix_node->is_terminal) { // Makes this model independent 
            matrix_node->inference.value_estimate0 = state.payoff0;
            matrix_node->inference.value_estimate1 = state.payoff1;
        } else {
            model.inference(state, matrix_node->legal_actions);
            matrix_node->inference = model.last_inference; // Inference expects a state, gets T instead...
        }

        matrix_node->stats.cumulative_payoffs.rows = matrix_node->legal_actions.rows; 
        matrix_node->stats.cumulative_payoffs.cols = matrix_node->legal_actions.cols;
        matrix_node->stats.visits.rows = matrix_node->legal_actions.rows;
        matrix_node->stats.visits.cols = matrix_node->legal_actions.cols;
        for (int row_idx = 0; row_idx < matrix_node->legal_actions.rows; ++row_idx) {
            for (int col_idx = 0; col_idx < matrix_node->legal_actions.cols; ++col_idx) {
                matrix_node->stats.cumulative_payoffs.set0(row_idx, col_idx, 0);
                matrix_node->stats.cumulative_payoffs.set1(row_idx, col_idx, 0);
                matrix_node->stats.visits.set(row_idx, col_idx, 0);
            }
        }

        // Uniform initialization of stats.strategies
        for (int row_idx = 0; row_idx < matrix_node->legal_actions.rows; ++row_idx) {matrix_node->stats.strategy0[row_idx] = 1 /(float)matrix_node->legal_actions.rows;}
        for (int col_idx = 0; col_idx < matrix_node->legal_actions.cols; ++col_idx) {matrix_node->stats.strategy1[col_idx] = 1 /(float)matrix_node->legal_actions.cols;}

        // time and mutex
        int parent_idx = -1;
        int prev_idx = -1;
        ChanceNode<MatrixUCB>* parent = matrix_node->parent;
        if (parent != nullptr) {
            MatrixNode<MatrixUCB>*  mparent = parent->parent;
            parent_idx = mparent->stats.mutex_idx;
            int row_idx = parent->row_idx;
            int col_idx = parent->col_idx;
            double joint_p = mparent->inference.strategy_prior0[row_idx]*mparent->inference.strategy_prior1[col_idx] * ((double) matrix_node->transition_data.probability);
            int t_estimate = matrix_node->parent->parent->stats.t * joint_p;
            t_estimate = t_estimate == 0 ? 1 : t_estimate;
            matrix_node->stats.t = t_estimate;
        }
        if (matrix_node->prev != nullptr) {
            prev_idx = matrix_node->prev->stats.mutex_idx;
        }
        while (true) {
            matrix_node->stats.mutex_idx = model.device.random_int(pool_size);
            if (matrix_node->stats.mutex_idx != parent_idx && matrix_node->stats.mutex_idx != prev_idx) {
                break;
            }
        }
    }

    MatrixNode<MatrixUCB>* runPlayout ( 
        typename MatrixUCB::state_t& state, 
        typename MatrixUCB::model_t& model, 
        MatrixNode<MatrixUCB>* matrix_node
    ) {

    std::mutex& mtx = mutex_pool[matrix_node->stats.mutex_idx];

        if (matrix_node->is_terminal == true) {

            return matrix_node;
        } else {
        mtx.lock();
            if (matrix_node->is_expanded == true) {
                Linear::Bimatrix2D<double, MatrixUCB::state_t::size_> A(matrix_node->legal_actions.rows, matrix_node->legal_actions.cols);
                std::array<double, MatrixUCB::state_t::size_> strategy0 = {1/(double)matrix_node->legal_actions.rows};
                std::array<double, MatrixUCB::state_t::size_> strategy1 = {1/(double)matrix_node->legal_actions.cols};
                process_matrix(
                    matrix_node->stats.cumulative_payoffs,
                    matrix_node->stats.visits,
                    A,
                    matrix_node->stats.t
                );
        mtx.unlock();
                double exploitability = Bandit::exploitability<double, MatrixUCB::state_t::size_>(
                    A, 
                    strategy0, 
                    strategy1
                );
                if (exploitability > .05) {
                    Bandit::SolveBimatrix<double, MatrixUCB::state_t::size_> (
                        device,
                        10000,
                        A,
                        strategy0,
                        strategy1
                    );
                }

    // std::cout<<std::endl;
    // std::cout << "Time " << matrix_node->stats.t << std::endl;
    // std::cout << "Payoff" << std::endl;
    // matrix_node->stats.cumulative_payoffs.print();
    // std::cout << "Visits" << std::endl;
    // matrix_node->stats.visits.print();
    // std::cout << "M" << std::endl;
    // (matrix_node->stats.cumulative_payoffs + matrix_node->stats.visits).print();
    // std::cout << "A" << std::endl;
    // A.print();  
    // std::cout << "Strategies" << std::endl;
    // std::cout << matrix_node->stats.strategy0[0] << ' ' << matrix_node->stats.strategy0[1] << std::endl;
    // std::cout << matrix_node->stats.strategy1[0] << ' ' << matrix_node->stats.strategy1[1] << std::endl;
    // std::cout << "expl: " << exploitability << std::endl;

                int row_idx = device.sample_pdf<double, MatrixUCB::state_t::size_>(matrix_node->stats.strategy0, matrix_node->legal_actions.rows);
                int col_idx = device.sample_pdf<double, MatrixUCB::state_t::size_>(matrix_node->stats.strategy1, matrix_node->legal_actions.cols);
                typename MatrixUCB::action_t action0 = matrix_node->legal_actions.actions0[row_idx];
                typename MatrixUCB::action_t action1 = matrix_node->legal_actions.actions1[col_idx];
                typename MatrixUCB::transition_data_t transition_data = state.apply_actions(action0, action1);

                ChanceNode<MatrixUCB>* chance_node = matrix_node->access(row_idx, col_idx);
                MatrixNode<MatrixUCB>* matrix_node_next = chance_node->access(transition_data);
                    MatrixNode<MatrixUCB>* matrix_node_leaf = runPlayout(state, model, matrix_node_next);

                double u0 = matrix_node_leaf->inference.value_estimate0;
                double u1 = matrix_node_leaf->inference.value_estimate1;
        mtx.lock();
                update(matrix_node, u0, u1, row_idx, col_idx);
                for (int i = 0; i < 2; ++i) {
                    matrix_node->stats.strategy0[i] = strategy0[i];
                    matrix_node->stats.strategy1[i] = strategy1[i];
                }
                // matrix_node->stats.strategy0 = strategy0;
                // matrix_node->stats.strategy1 = strategy1;
        mtx.unlock();                
                return matrix_node_leaf;
            } else {
                expand(state, model, matrix_node);
        mtx.unlock();
                return matrix_node;
            }
        }
    };

    void loopPlayout (
        int playouts,
        typename MatrixUCB::state_t* state, 
        MatrixNode<MatrixUCB>* matrix_node
    ) {
        prng device_(device.random_int(2147483647));
        typename MatrixUCB::model_t model(device_);
        
        for (int playout = 0; playout < playouts; ++playout) {
            auto state_ = *state;
            runPlayout(state_, model, matrix_node);
        }
    }


    void search (
        int threads, 
        int playouts, 
        typename MatrixUCB::state_t& state, 
        MatrixNode<MatrixUCB>* root
    ) {
        root->stats.t = playouts;
        std::thread thread_pool[threads];

        for (int i = 0; i < threads; ++i) {
            const int playouts_thread = playouts / threads;
            thread_pool[i] = std::thread(&MatrixUCB::loopPlayout, this, playouts_thread, &state, root);
        }
        for (int i = 0; i < threads; ++i) {
            thread_pool[i].join();
        }

        // TODO remove this
        Linear::Bimatrix2D<double, MatrixUCB::state_t::size_> A(root->legal_actions.rows, root->legal_actions.cols);
        process_matrix_final(
            root->stats.cumulative_payoffs,
            root->stats.visits,
            A,
            root->stats.t
        );
        
        A.print();

        Bandit::SolveBimatrix<double, MatrixUCB::state_t::size_> (
            device,
            10000,
            A,
            root->stats.strategy0,
            root->stats.strategy1
        );

        // std::cout << root->stats.strategy0[0] << ' ' << root->stats.strategy0[1] << std::endl;
        // std::cout << root->stats.strategy1[0] << ' ' << root->stats.strategy1[1] << std::endl;

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
        Linear::Bimatrix<double, MatrixUCB::state_t::size_>& output,
        int t
    ) {
        // assert dimensions make sense TODO
        // rename vars TODO
        const int rows = output.rows;
        const int cols = output.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx) {
            for (int col_idx = 0; col_idx < cols; ++col_idx) {
                double u = cumulative_payoffs.get0(row_idx, col_idx);
                double v = cumulative_payoffs.get1(row_idx, col_idx);
                int n = visits.get(row_idx, col_idx);
                n += 1;
                double a = n > 0 ? u / n : .5;
                double b = n > 0 ? v / n : .5;
                double const c = 1;
                double const eta = c * std::sqrt((2 * std::log(t) + std::log(2 * rows * cols)) / n);
                const double x = a + eta;
                const double y = b + eta;
                output.set0(row_idx, col_idx, x);
                output.set1(row_idx, col_idx, y);
            }
        }
    }

    void process_matrix_final (
        Linear::Bimatrix<double, MatrixUCB::state_t::size_>& cumulative_payoffs, 
        Linear::Matrix<int, MatrixUCB::state_t::size_>& visits, 
        Linear::Bimatrix<double, MatrixUCB::state_t::size_>& output,
        int t
    ) {
        // assert dimensions make sense TODO
        // rename vars TODO
        const int rows = output.rows;
        const int cols = output.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx) {
            for (int col_idx = 0; col_idx < cols; ++col_idx) {
                double u = cumulative_payoffs.get0(row_idx, col_idx);
                double v = cumulative_payoffs.get1(row_idx, col_idx);
                int n = visits.get(row_idx, col_idx);
                n += 1;
                double a = n > 0 ? u / n : .5;
                double b = n > 0 ? v / n : .5;
                output.set0(row_idx, col_idx, a);
                output.set1(row_idx, col_idx, b);
            }
        }
    }
};