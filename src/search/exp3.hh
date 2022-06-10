#pragma once

#include "tree/node.hh"

template <int size>
struct Exp3Stats :  stats {

    std::array<float, size> gains0;
    std::array<float, size> gains1;
    std::array<int, size> visits0;
    std::array<int, size> visits1;

};

struct SearchSession {
    prng& device;
    SearchSession (prng& device) : device(device) {}
};


// Exp3 


template <int size>
struct Exp3SearchSession : SearchSession {

    float eta = .01;

    Exp3SearchSession<size> (prng& device) :
        SearchSession(device) {}
    Exp3SearchSession<size> (prng& device, float eta) :
        SearchSession(device), eta(eta) {}

    void expand (MatrixNode<size, Exp3Stats<size>>* matrix_node, State<size>& state, Model<size>& model);
    MatrixNode<size, Exp3Stats<size>>* search (MatrixNode<size, Exp3Stats<size>>* matrix_node_current, State<size>& state, Model<size>& model);
    void forecast (std::array<float, size>& forecast0, std::array<float, size>& forecast1, MatrixNode<size, Exp3Stats<size>>* matrix_node);
};

template<int size>
void Exp3SearchSession<size> :: expand (MatrixNode<size, Exp3Stats<size>>* matrix_node, State<size>& state, Model<size>& model) {

    state.actions(matrix_node->pair);

    matrix_node->terminal = (matrix_node->pair.rows*matrix_node->pair.cols == 0);
    
    if (matrix_node->terminal) {
        matrix_node->inference.value_estimate0 = state.payoff;
        matrix_node->inference.value_estimate1 = 1 - state.payoff;
        return;
    }

    matrix_node->inference = model.inference(state); // test that this copies correctyly!!!!

    matrix_node->expanded = true;
} 


template <int size>
MatrixNode<size, Exp3Stats<size>>* Exp3SearchSession<size> :: search (
    MatrixNode<size, Exp3Stats<size>>* matrix_node_current, 
    State<size>& state,
    Model<size>& model) {

    if (matrix_node_current->terminal == true) {
        return matrix_node_current;
    } else {
        if (matrix_node_current->expanded == true) {
            std::array<float, size> forecast0;
            std::array<float, size> forecast1;
            forecast(forecast0, forecast1, matrix_node_current);

            if (device.uniform() < .000001) {
                for (int idx = 0; idx < size; ++idx) {
                   std::cout << forecast0[idx] << ' ';
                }
                std::cout << std::endl;
            }

            int row_idx = device.sample_pdf<float, size>(forecast0, matrix_node_current->pair.rows);
            int col_idx = device.sample_pdf<float, size>(forecast1, matrix_node_current->pair.cols);

            //std::cout << row_idx << ' ' << col_idx << std::endl;

            StateTransitionData transition_data = state.transition(row_idx, col_idx);
            // std::cout << '!' << std::endl;
            ChanceNode<size, Exp3Stats<size>>* chance_node = matrix_node_current->access(row_idx, col_idx);
            MatrixNode<size, Exp3Stats<size>>* matrix_node_next = chance_node->access(transition_data);
            MatrixNode<size, Exp3Stats<size>>* matrix_node_leaf = Exp3SearchSession::search(matrix_node_next, state, model);

            float u0 = matrix_node_leaf->inference.value_estimate0;
            float u1 = matrix_node_leaf->inference.value_estimate1;

            // //std::cout << u0 << ',' << u1 << std::endl;

            // Causes an update of the selection probs for next round
            matrix_node_current->s.gains0[row_idx] += u0 / forecast0[row_idx];
            matrix_node_current->s.gains1[col_idx] += u1 / forecast1[col_idx];

            // // Stats
            matrix_node_current->s.visits0[row_idx] += 1;
            matrix_node_current->s.visits1[col_idx] += 1;
            matrix_node_current->update(u0, u1);
            chance_node->update(u0, u1);

            // return matrix_node_leaf;
            return matrix_node_leaf;
        } else {
            expand(matrix_node_current, state, model);
            return matrix_node_current;
        }
    }
};



template <int size>
void Exp3SearchSession<size> :: forecast (
    std::array<float, size>& forecast0, 
    std::array<float, size>& forecast1, 
    MatrixNode<size, Exp3Stats<size>>* matrix_node) {

    float max = 0;
    for (int i = 0; i < matrix_node->pair.rows; ++i) {
        float x = matrix_node->s.gains0[i];
        if (x > max) {
            max = x;
        } 
    }
    float sum = 0;
    for (int i = 0; i < matrix_node->pair.rows; ++i) {
        matrix_node->s.gains0[i] -= max;
        float x = matrix_node->s.gains0[i];
        float y = std::exp(x * eta / matrix_node->pair.rows);
        forecast0[i] = y;
        sum += y;
    }
    for (int i = 0; i < matrix_node->pair.rows; ++i) {
        forecast0[i] *= (1-eta)/sum;
        forecast0[i] += eta/matrix_node->pair.rows;
    }

    max = 0;
    for (int i = 0; i < matrix_node->pair.cols; ++i) {
        float x = matrix_node->s.gains1[i];
        if (x > max) {
            max = x;
        } 
    }
    sum = 0;
    for (int i = 0; i < matrix_node->pair.cols; ++i) {
        matrix_node->s.gains1[i] -= max;
        float x = matrix_node->s.gains1[i];
        float y = std::exp(x * eta / matrix_node->pair.cols);
        forecast1[i] = y;
        sum += y;
    }
    for (int i = 0; i < matrix_node->pair.cols; ++i) {
        forecast1[i] *= (1-eta)/sum;
        forecast1[i] += eta/matrix_node->pair.cols;
    }
}