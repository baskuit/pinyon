#include "../libsurskit/math.hh"
#include "exp3.hh"
#include "../model/monte_carlo.hh"

#include <iostream>

MatrixNode* Exp3SearchSession::search (MatrixNode* matrix_node_current, State* state) {

    if (matrix_node_current->terminal == true) {

        return matrix_node_current;
    } else {
        if (matrix_node_current->expanded == true) {
            float forecasts0[matrix_node_current->rows];
            float forecasts1[matrix_node_current->cols];
            forecast(forecasts0, matrix_node_current->gains0, matrix_node_current->rows);
            forecast(forecasts1, matrix_node_current->gains1, matrix_node_current->cols);
            int row_idx = math::sample(forecasts0, matrix_node_current->rows);
            int col_idx = math::sample(forecasts1, matrix_node_current->cols);

            StateTransitionData data = state->transition(row_idx, col_idx);

            ChanceNode* chance_node = matrix_node_current->access(row_idx, col_idx);
            MatrixNode* matrix_node_next = chance_node->access(data.transitionKey, data.transitionProb);
            MatrixNode* matrix_node_leaf = Exp3SearchSession::search(matrix_node_next, state);

            float u0 = matrix_node_leaf->value_estimate0;
            float u1 = matrix_node_leaf->value_estimate1;

            //std::cout << u0 << ',' << u1 << std::endl;

            // Causes an update of the selection probs for next round
            matrix_node_current->gains0[row_idx] += u0 / forecasts0[row_idx];
            matrix_node_current->gains1[col_idx] += u1 / forecasts1[col_idx];

            // Stats
            matrix_node_current->visits0[row_idx] += 1;
            matrix_node_current->visits1[col_idx] += 1;
            matrix_node_current->cumulative_score0 += u0;
            matrix_node_current->cumulative_score1 += u1;
            chance_node->cumulative_score0 += u0;
            chance_node->cumulative_score1 += u1; 
            matrix_node_current->visits += 1;
            chance_node->visits += 1;

            return matrix_node_leaf;
        } else {
            matrix_node_current->expand(state, this->model);

            return matrix_node_current;
        }
    }
}

void Exp3SearchSession::search (int playouts) {
    
    for (int playout = 0; playout < playouts; ++playout) {
        State* state_ = this->state->copy();
        Exp3SearchSession::search(this->root, state_);
        delete state_;

    }
    this->playouts += playouts;
    visits0 = root->visits0;
    visits1 = root->visits0;
    cumulative_score0 = root->cumulative_score0;
    cumulative_score1 = root->cumulative_score1;
}

// should return float strategies without the 'uniform noise'
void Exp3SearchSession::denoise () {
    int rows = this->root->rows;
    int cols = this->root->cols;

    this->nash_solution0 = new float[rows]{0.f};
    this->nash_solution1 = new float[cols]{0.f};
    for (int i = 0; i < rows; ++i) {
        nash_solution0[i] = this->visits0[i]/ (float)this->playouts;
    }
    for (int j = 0; j < cols; ++j) {
        nash_solution1[j] = this->visits1[j]/ (float)this->playouts;
    }
    float eta_ = 1/(1 - this->eta);
    for (int i = 0; i < rows; ++i) {
        nash_solution0[i] -= (this->eta/(float)rows);
        nash_solution0[i] *= eta_;
        std::cout << nash_solution0[i] << ", " << std::endl;
    }
    for (int j = 0; j < cols; ++j) {
        nash_solution1[j] -= (this->eta/(float)cols);
        nash_solution1[j] *= eta_;
        std::cout << nash_solution1[j] << ", " << std::endl;
    }
};

void Exp3SearchSession::forecast(float* forecasts, float* gains, int k) {
    float max = 0;
    for (int i = 0; i < k; ++i) {
        float x = gains[i];
        if (x > max) {
            max = x;
        } 
    }
    float sum = 0;
    for (int i = 0; i < k; ++i) {
        gains[i] -= max;
        float x = gains[i];
        float y = std::exp(x * this->eta / k);
        forecasts[i] = y;
        sum += y;
    }
    for (int i = 0; i < k; ++i) {
        forecasts[i] *= (1-this->eta)/sum;
        forecasts[i] += this->eta/k;
    }
}
