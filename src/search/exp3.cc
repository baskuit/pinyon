#include "../libsurskit/math.hh"
#include "../search/exp3.hh"
#include "../model/monte_carlo.hh"

class Exp3SearchSession : public SearchSession {
public:
    float eta = 0.01f;

    MatrixNode* search (MatrixNode* matrix_node_current, State* state, Model* model) {

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
                MatrixNode* matrix_node_leaf = search(matrix_node_next, state, model);

                float u0 = matrix_node_leaf->value_estimate0;
                float u1 = matrix_node_leaf->value_estimate1;

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
                matrix_node_current->expand(state, model);

                return matrix_node_current;
            }
        }
    }

    void search (int playouts) {
        this->playouts += playouts;
        for (int playout = 0; playout < playouts; ++playout) {
            search(root, state, model);
        }
    }

    // should return float strategies without the 'uniform noise'
    void denoise () {};

private:

    void forecast(float* forecasts, float* gains, int k) {
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
            float y = std::exp(x * eta / k);
            forecasts[i] = y;
            sum += y;
        }
        for (int i = 0; i < k; ++i) {
            forecasts[i] *= (1-eta)/sum;
            forecasts[i] += eta/k;
        }
    }

};