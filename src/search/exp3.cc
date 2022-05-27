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

            std::cout << u0 << ' ' << u1 << std::endl;

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
        MatrixNode* matrix_node = Exp3SearchSession::search(this->root, state_);
        delete state_;
        while (matrix_node->parent && matrix_node->parent->parent != this->root) {
            matrix_node = matrix_node->parent->parent;
        }
        Action action0, action1;
        if (matrix_node != this->root) {
            // not the one expansion term, and not terminal
            action0 = matrix_node->parent->action0;
            action1 = matrix_node->parent->action1;
        }
        for (int row_idx = 0; row_idx < this->root->rows; ++row_idx) {
            if (this->root->actions0[row_idx] == action0) {
                this->visits0[row_idx] += 1;
            }
        }
        for (int col_idx = 0; col_idx < this->root->rows; ++col_idx) {
            if (this->root->actions1[col_idx] == action1) {
                this->visits1[col_idx] += 1;
            }
        }
        cumulativeScore0 += matrix_node->value_estimate0;
        cumulativeScore1 += matrix_node->value_estimate1;
    }
}


SearchSessionData Exp3SearchSession::answer () {
    SearchSessionData data;
    int rows = this->root->rows;
    int cols = this->root->cols;
    data.playouts = this->playouts;
    data.rows = rows;
    data.cols = cols;
    data.cumulative_score0 = this->cumulativeScore0;
    data.cumulative_score1 = this->cumulativeScore1;

    // convert vists to prob distro and denoise
    data.nash_solution0 = new float[rows]{0.f};
    data.nash_solution1 = new float[cols]{0.f};
    const float playouts_ = this->playouts - 1;
    for (int row_idx = 0; row_idx < rows; ++row_idx) {
        data.nash_solution0[row_idx] = this->visits0[row_idx]/ playouts_;
        data.visits0[row_idx] = this->visits0[row_idx];
    }
    for (int col_idx = 0; col_idx < cols; ++col_idx) {
        data.nash_solution1[col_idx] = this->visits1[col_idx]/ playouts_;
        data.visits1[col_idx] = this->visits1[col_idx];
    }
    const float eta_ = 1/(1 - this->eta);
    float row_sum = 0;
    for (int row_idx = 0; row_idx < rows; ++row_idx) {
        data.nash_solution0[row_idx] -= (this->eta/(float)rows);
        data.nash_solution0[row_idx] *= data.nash_solution0[row_idx] > 0 ? eta_ : 0;
        row_sum += data.nash_solution0[row_idx];
    }
    float col_sum = 0;
    for (int col_idx = 0; col_idx < cols; ++col_idx) {
        data.nash_solution1[col_idx] -= (this->eta/(float)cols);
        data.nash_solution1[col_idx] *= data.nash_solution1[col_idx] > 0 ? eta_ : 0;
        col_sum += data.nash_solution1[col_idx];
    }

    // print
    for (int row_sum = 0; row_sum < rows; ++row_sum) {
        data.nash_solution0[row_sum] /= row_sum;
        std::cout << data.nash_solution0[row_sum] << ", ";
    }
    std::cout << std::endl;
    for (int col_idx = 0; col_idx < cols; ++col_idx) {
        data.nash_solution1[col_idx] /= col_sum;
        std::cout << data.nash_solution1[col_idx] << ", ";
    }
    std::cout << std::endl;

    return data;
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
