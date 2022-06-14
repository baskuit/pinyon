#pragma once

#include "session.hh"

template <int size>
struct Exp3pStats :  stats {

    int time = 0;
    std::array<double, size> gains0 = {0};
    std::array<double, size> gains1 = {0};
    // std::array<int, size> visits0;
    // std::array<int, size> visits1;
};

template <int size>
struct Exp3pAnswer {
    int time = 0;
    double value0 = .5f;
    double value1 = .5f;
    std::array<double, size> strategy0 = {0};
    std::array<double, size> strategy1 = {0};
    std::array<int, size> visits0 = {0};
    std::array<int, size> visits1 = {0};

    Exp3pAnswer () {}
    void print () {
        std::cout << "time: " << time << std::endl;
        std::cout << "s0: ";
        for (int i = 0; i < size; ++i) {
            std::cout << strategy0[i] << ' ';
        }
        std::cout << std::endl;
        std::cout << "v0: ";
        for (int i = 0; i < size; ++i) {
            std::cout << visits0[i] << ' ';
        }
        std::cout << std::endl;
        std::cout << "s1: ";
        for (int i = 0; i < size; ++i) {
            std::cout << strategy1[i] << ' ';
        }
        std::cout << std::endl;
        std::cout << "v1: ";
        for (int i = 0; i < size; ++i) {
            std::cout << visits1[i] << ' ';
        }
        std::cout << std::endl;
        std::cout << "u0: " << value0 << " u1: " << value1 << std::endl;
    }
};




template <int size, typename T>
struct Exp3pSearchSession : SearchSession {

    MatrixNode<size, Exp3pStats<size>>* root; 
    T& state;
    Model<size>& model;

    int time = 0;
    std::array<int, size> visits0 = {0};
    std::array<int, size> visits1 = {0};
    double cumulative_value0 = 0;
    double cumulative_value1 = 0;

    Exp3pSearchSession<size, T> (
    prng& device, 
    T& state, 
    Model<size>& model, 
    MatrixNode<size, Exp3pStats<size>>* root, 
    int time) :
        SearchSession(device), root(root), state(state), model(model), time(time) {
        if (!root->expanded) {
            T state_ = state;
            expand(time, root, state_);
        }
    }

    // Infernces the state with the model, and applies that information to the matrix node
    void expand (int time, MatrixNode<size, Exp3pStats<size>>* matrix_node, T& state);

    // Returns the leaf node of a playout. Either game-terminal, pruned-terminal, or just now expanded.
    MatrixNode<size, Exp3pStats<size>>* search (MatrixNode<size, Exp3pStats<size>>* matrix_node_current, T& state);

    //  Public interface for other search. Updates session statistics for this->answer().
    void search (int time) {
        this->time += time;
        for (int playout = 0; playout < time; ++ playout) {
            T state_ = state;
            MatrixNode<9 , Exp3pStats<9>>* leaf = this->search(root, state_);

            cumulative_value0 += leaf->inference.value_estimate0;
            cumulative_value1 += leaf->inference.value_estimate1;

            // We are deducing the actions played from outside the search(Node*, State&) function
            // So we have do this little rigmarol of tracing back from the leaf 

            if (leaf != root) {
                while (leaf->parent->parent != root) {
                    leaf = leaf->parent->parent;
                }
                auto a0 = leaf->parent->action0;
                auto a1 = leaf->parent->action1;
                int row_idx = 0;
                int col_idx = 0;
                for (int i = 0; i < root->pair.rows; ++i) {
                    if (a0 == root->pair.actions0[i]) {
                        row_idx = i;
                    }
                }
                for (int j = 0; j < root->pair.cols; ++j) {
                    if (a1 == root->pair.actions1[j]) {
                        col_idx = j;
                    }
                }
                visits0[row_idx] += 1;
                visits1[col_idx] += 1;
            }
        }
    };

    // Softmax and uniform noise
    void softmax (std::array<double, size>& forecast, MatrixNode<size, Exp3pStats<size>>* matrix_node, double eta);

    void forecast (std::array<double, size>& forecast0, std::array<double, size>& forecast1, MatrixNode<size, Exp3pStats<size>>* matrix_node);

    // Returned denoised empirical strategies collected over the session (not over the node's history)
    Exp3pAnswer<size> answer ();
};







// Implementation ------------------------------------------------------







template<int size, typename T>
void Exp3pSearchSession<size, T> :: expand (int time, MatrixNode<size, Exp3pStats<size>>* matrix_node, T& state) {
    matrix_node->expanded = true;

    matrix_node->inference = model.inference(state, matrix_node->pair); // Inference expects a state, gets T instead...
    matrix_node->terminal = (matrix_node->pair.rows*matrix_node->pair.cols == 0);
    
    if (matrix_node->terminal) {
        matrix_node->inference.value_estimate0 = state.payoff;
        matrix_node->inference.value_estimate1 = 1 - state.payoff;
    }

    matrix_node->s.time = time;
} 



template<int size, typename T>
MatrixNode<size, Exp3pStats<size>>* Exp3pSearchSession<size, T> :: search (
    MatrixNode<size, Exp3pStats<size>>* matrix_node_current, 
    T& state) {

    if (matrix_node_current->terminal == true) {
        return matrix_node_current;
    } else {
        if (matrix_node_current->expanded == true) {
            std::array<double, size> forecast0;
            std::array<double, size> forecast1;
            forecast(forecast0, forecast1, matrix_node_current);

            int row_idx = device.sample_pdf<double, size>(forecast0, matrix_node_current->pair.rows);
            int col_idx = device.sample_pdf<double, size>(forecast1, matrix_node_current->pair.cols);
            Action action0 = matrix_node_current->pair.actions0[row_idx];
            Action action1 = matrix_node_current->pair.actions1[col_idx];
            StateTransitionData transition_data = state.transition(action0, action1);

            ChanceNode<size, Exp3pStats<size>>* chance_node = matrix_node_current->access(row_idx, col_idx);
            MatrixNode<size, Exp3pStats<size>>* matrix_node_next = chance_node->access(transition_data);
            MatrixNode<size, Exp3pStats<size>>* matrix_node_leaf = search(matrix_node_next, state);

            double u0 = matrix_node_leaf->inference.value_estimate0;
            double u1 = matrix_node_leaf->inference.value_estimate1;
            matrix_node_current->s.gains0[row_idx] += u0 / forecast0[row_idx];
            matrix_node_current->s.gains1[col_idx] += u1 / forecast1[col_idx];
            matrix_node_current->update(u0, u1);
            chance_node->update(u0, u1);

            return matrix_node_leaf;
        } else {
            expand(matrix_node_current->s.time, matrix_node_current, state);
            return matrix_node_current;
        }
    }
};



template<int size, typename T>
void Exp3pSearchSession<size, T> :: softmax (
    std::array<double, size>& forecast, 
    MatrixNode<size, Exp3pStats<size>>* matrix_node,
    double eta) {

    double max = 0;
    for (int i = 0; i < matrix_node->pair.rows; ++i) {
        double x = matrix_node->s.gains0[i];
        if (x > max) {
            max = x;
        } 
    }
    double sum = 0;
    for (int i = 0; i < matrix_node->pair.rows; ++i) {
        matrix_node->s.gains0[i] -= max;
        double x = matrix_node->s.gains0[i];
        double y = std::exp(x * eta);
        forecast[i] = y;
        sum += y;
    }
    for (int i = 0; i < matrix_node->pair.rows; ++i) {
        forecast[i] /= sum;
    }
}



template <int size, typename T>
void Exp3pSearchSession<size, T> :: forecast (
    std::array<double, size>& forecast0, 
    std::array<double, size>& forecast1,
    MatrixNode<size, Exp3pStats<size>>* matrix_node
) {
    const int time = matrix_node->s.time;
    const int rows = matrix_node->pair.rows;
    const int cols = matrix_node->pair.cols;
    if (rows == 1) {
        forecast0[0] = 1;
    } else {
        const double eta = .95 * sqrt(log(rows)/(time*rows));
        const double gamma_ = 1.05 * sqrt(rows*log(rows)/time);
        const double gamma = gamma_ < 1 ? gamma_ : 1;
        softmax(forecast0, matrix_node, eta);
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
        softmax(forecast1, matrix_node, eta);
        for (int col_idx = 0; col_idx < cols; ++col_idx) {
            double x = (1 - gamma) * forecast1[col_idx] + (gamma) * matrix_node->inference.strategy_prior0[col_idx];
            forecast1[col_idx] = x;
        }
    }
}



template<int size, typename T>
Exp3pAnswer<size> Exp3pSearchSession<size, T> :: answer () {
    Exp3pAnswer<size> data;
    const int rows = root->pair.rows;
    const int cols = root->pair.cols;
    for (int i = 0; i < rows; ++i) {
        data.strategy0[i] = visits0[i]/ (double)time;
    }
    for (int j = 0; j < cols; ++j) {
        data.strategy1[j] = visits1[j]/ (double)time;
    }
    const double eta_ = 1;
    double row_sum = 0;
    for (int i = 0; i < rows; ++i) {
        //data.strategy0[i] -= (eta/(double)rows);
        data.strategy0[i] *= data.strategy0[i] > 0 ? eta_ : 0;
        row_sum += data.strategy0[i];
    }
    double col_sum = 0;
    for (int j = 0; j < cols; ++j) {
        //data.strategy1[j] -= (eta/(double)cols);
        data.strategy1[j] *= data.strategy1[j] > 0 ? eta_ : 0;
        col_sum += data.strategy1[j];
    }
    for (int i = 0; i < rows; ++i) {
        data.visits0[i] = visits0[i];
        data.strategy0[i] /= row_sum;
    }
    for (int j = 0; j < cols; ++j) {
        data.visits1[j] = visits1[j];
        data.strategy1[j] /= col_sum;
    }
    data.value0 = cumulative_value0 / time;
    data.value1 = cumulative_value1 / time;
    data.time = time;

    return data;
};