#pragma once

#include "tree/node.hh"

template <int size>
struct Exp3Stats :  stats {

    std::array<float, size> gains0;
    std::array<float, size> gains1;
    std::array<int, size> visits0;
    std::array<int, size> visits1;

};

struct SearchSession {};


// Exp3 


template <int size>
struct Exp3SearchSession : SearchSession {

    float eta = .01;

    void expand (MatrixNode<size, Exp3Stats<size>>& matrix_node, State<size>& state, Model<size>& model);
    MatrixNode<size, Exp3Stats<size>>& search (MatrixNode<size, Exp3Stats<size>>& matrix_node_current, State<size>& state, Model<size>& model);
    void forecast (std::array<float, size>& forecast0, std::array<float, size>& forecast1, MatrixNode<size, Exp3Stats<size>>& matrix_node);
};

template<int size>
void Exp3SearchSession<size> :: expand (MatrixNode<size, Exp3Stats<size>>& matrix_node, State<size>& state, Model<size>& model) {

    state.actions(matrix_node.pair);

    matrix_node.terminal = (matrix_node.pair.rows*matrix_node.pair.cols == 0);
    
    if (matrix_node.terminal) {
        matrix_node.inference.value_estimate0 = state.payoff;
        matrix_node.inference.value_estimate1 = 1 - state.payoff;
        return;
    }

    matrix_node.inference = model.inference(state); // test that this copies correctyly!!!!

    matrix_node.expanded = true;
} 


template <int size>
MatrixNode<size, Exp3Stats<size>>& Exp3SearchSession<size> :: search (
    MatrixNode<size, Exp3Stats<size>>& matrix_node_current, 
    State<size>& state,
    Model<size>& model) {

    if (matrix_node_current.terminal == true) {
        return matrix_node_current;
    } else {
        if (matrix_node_current.expanded == true) {
            // float forecasts0[matrix_node_current.rows];
            // float forecasts1[matrix_node_current.cols];
            // forecast(forecasts0, matrix_node_current.data, matrix_node_current.rows);
            // forecast(forecasts1, matrix_node_current.gains1, matrix_node_current.cols);
            // int row_idx = math::sample(forecasts0, matrix_node_current.rows);
            // int col_idx = math::sample(forecasts1, matrix_node_current.cols);

            // StateTransitionData data = state.transition(row_idx, col_idx);

            // ChanceNode* chance_node = *matrix_node_current.access(row_idx, col_idx);
            // MatrixNode matrix_node_next = *chance_node.access(data.transitionKey, data.transitionProb);
            // MatrixNode matrix_node_leaf = Exp3SearchSession::search(matrix_node_next, state);

            // float u0 = matrix_node_leaf.value_estimate0;
            // float u1 = matrix_node_leaf.value_estimate1;

            // //std::cout << u0 << ',' << u1 << std::endl;

            // // Causes an update of the selection probs for next round
            // matrix_node_current.gains0[row_idx] += u0 / forecasts0[row_idx];
            // matrix_node_current.gains1[col_idx] += u1 / forecasts1[col_idx];

            // // Stats
            // matrix_node_current.visits0[row_idx] += 1;
            // matrix_node_current.visits1[col_idx] += 1;
            // matrix_node_current.cumulative_score0 += u0;
            // matrix_node_current.cumulative_score1 += u1;
            // chance_node.cumulative_score0 += u0;
            // chance_node.cumulative_score1 += u1; 
            // matrix_node_current.visits += 1;
            // chance_node.visits += 1;

            // return matrix_node_leaf;
            return matrix_node_current;
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
    MatrixNode<size, Exp3Stats<size>>& matrix_node) {

    float max = 0;
    for (int i = 0; i < matrix_node.pair.rows; ++i) {
        float x = matrix_node.s.gains0[i];
        if (x > max) {
            max = x;
        } 
    }
    float sum = 0;
    for (int i = 0; i < matrix_node.pair.rows; ++i) {
        matrix_node.s.gains0[i] -= max;
        float x = matrix_node.s.gains0[i];
        float y = std::exp(x * eta / matrix_node.pair.rows);
        forecast0[i] = y;
        sum += y;
    }
    for (int i = 0; i < matrix_node.pair.rows; ++i) {
        forecast0[i] *= (1-eta)/sum;
        forecast0[i] += eta/matrix_node.pair.rows;
    }

    max = 0;
    for (int i = 0; i < matrix_node.pair.cols; ++i) {
        float x = matrix_node.s.gains1[i];
        if (x > max) {
            max = x;
        } 
    }
    sum = 0;
    for (int i = 0; i < matrix_node.pair.cols; ++i) {
        matrix_node.s.gains1[i] -= max;
        float x = matrix_node.s.gains1[i];
        float y = std::exp(x * eta / matrix_node.pair.cols);
        forecast1[i] = y;
        sum += y;
    }
    for (int i = 0; i < matrix_node.pair.cols; ++i) {
        forecast1[i] *= (1-eta)/sum;
        forecast1[i] += eta/matrix_node.pair.cols;
    }
}