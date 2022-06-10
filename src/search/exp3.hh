#pragma once

#include "tree/node.hh"

template <int size>
struct Exp3Stats {

    std::array<float, size> gains0;
    std::array<float, size> gains1;
    std::array<int, size> visits0;
    std::array<int, size> visits1;

};

// template <int size, typename stats>
// struct Exp3stats<size, stats> : stats {
// public:
//     int* visits0 = nullptr;
//     int* visits1 = nullptr;
//     float* gains0 = nullptr;
//     float* gains1 = nullptr;

//     Exp3stats () {}
//     Exp3stats (int* visits0, int* visits1, float* gains0, float* gains1) :
//         visits0(visits0), visits1(visits1), gains0(gains0), gains1(gains1) {}
//     Exp3stats (int rows, int cols) : 
//         visits0(new int[rows] {0}), visits1(new int[cols] {0}), gains0(new float[rows] {0.f}), gains1(new float[cols] {0.f}) {}
//     ~Exp3stats () {
//         delete [] visits0;
//         delete [] visits1;
//         delete [] gains0;
//         delete [] gains1;
//     }
// };

// // #include <iostream>

// void Exp3SearchSession :: expand (PairActions& pair, InferenceData& data, MatrixNode* matrix_node, State* state, Model* model) {

//     state->actions(pair);
//     matrix_node->rows = pair.rows;
//     matrix_node->cols = pair.cols;
    
//     matrix_node->actions0 = new Action[matrix_node->rows];
//     for (int row_idx = 0; row_idx < pair.rows; ++row_idx) {
//         matrix_node->actions0[row_idx] = pair.actions0[row_idx];
//     }
//     matrix_node->actions1 = new Action[matrix_node->cols];
//     for (int col_idx = 0; col_idx < pair.cols; ++col_idx) {
//         matrix_node->actions1[col_idx] = pair.actions1[col_idx];
//     }

//     matrix_node->terminal = (pair.rows*pair.cols == 0);
//     if (matrix_node->terminal) {
//         matrix_node->value_estimate0 = state->payoff;
//         matrix_node->value_estimate1 = 1 - state->payoff;
//         return;
//     }

//     model->inference(state, pair, data);
//     matrix_node->value_estimate0 = data.value_estimate0;
//     matrix_node->value_estimate1 = data.value_estimate1;
//     //matrix_node->strategy_prior0 not used in exp3 :/

//     /*
//     matrix_node->gains0 = new float[matrix_node->rows]{0.f};
//     matrix_node->gains1 = new float[matrix_node->cols]{0.f};
//     matrix_node->visits0 = new int[matrix_node->rows]{0};
//     matrix_node->visits1 = new int[matrix_node->cols]{0};
//     */
//     matrix_node->expanded = true;
// } 

// MatrixNode* Exp3SearchSession::search (MatrixNode* matrix_node_current, State* state) {

//     if (matrix_node_current->terminal == true) {

//         return matrix_node_current;
//     } else {
//         if (matrix_node_current->expanded == true) {
//             float forecasts0[matrix_node_current->rows];
//             float forecasts1[matrix_node_current->cols];
//             forecast(forecasts0, matrix_node_current->data, matrix_node_current->rows);
//             forecast(forecasts1, matrix_node_current->gains1, matrix_node_current->cols);
//             int row_idx = math::sample(forecasts0, matrix_node_current->rows);
//             int col_idx = math::sample(forecasts1, matrix_node_current->cols);

//             StateTransitionData data = state->transition(row_idx, col_idx);

//             ChanceNode* chance_node = matrix_node_current->access(row_idx, col_idx);
//             MatrixNode* matrix_node_next = chance_node->access(data.transitionKey, data.transitionProb);
//             MatrixNode* matrix_node_leaf = Exp3SearchSession::search(matrix_node_next, state);

//             float u0 = matrix_node_leaf->value_estimate0;
//             float u1 = matrix_node_leaf->value_estimate1;

//             //std::cout << u0 << ',' << u1 << std::endl;

//             // Causes an update of the selection probs for next round
//             matrix_node_current->gains0[row_idx] += u0 / forecasts0[row_idx];
//             matrix_node_current->gains1[col_idx] += u1 / forecasts1[col_idx];

//             // Stats
//             matrix_node_current->visits0[row_idx] += 1;
//             matrix_node_current->visits1[col_idx] += 1;
//             matrix_node_current->cumulative_score0 += u0;
//             matrix_node_current->cumulative_score1 += u1;
//             chance_node->cumulative_score0 += u0;
//             chance_node->cumulative_score1 += u1; 
//             matrix_node_current->visits += 1;
//             chance_node->visits += 1;

//             return matrix_node_leaf;
//         } else {
//             matrix_node_current->expand(state, this->model);

//             return matrix_node_current;
//         }
//     }
// }