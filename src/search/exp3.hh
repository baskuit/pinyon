#pragma once

#include "tree/node.hh"

template <int size>
struct Exp3Stats :  stats {

    std::array<double, size> gains0;
    std::array<double, size> gains1;
    // std::array<int, size> visits0;
    // std::array<int, size> visits1;

};

struct SearchSession {
    prng& device;
    SearchSession (prng& device) : device(device) {}
};


// Exp3 


template <int size>
struct Exp3Answer {
    int playouts = 0;
    double value0 = .5f;
    double value1 = .5f;
    std::array<double, size> strategy0 = {0};
    std::array<double, size> strategy1 = {0};
    std::array<int, size> visits0 = {0};
    std::array<int, size> visits1 = {0};

    Exp3Answer () {}
    void print () {
        std::cout << "playouts: " << playouts << std::endl;
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
struct Exp3SearchSession : SearchSession {

    double eta = .01;

    MatrixNode<size, Exp3Stats<size>>* root; 
    T& state;
    Model<size>& model; // TODO: Constructors, change function arguments.

    int playouts = 0;
    std::array<int, size> visits0 = {0};
    std::array<int, size> visits1 = {0};
    double cumulative_value0 = 0;
    double cumulative_value1 = 0;

    // Exp3SearchSession<size> (prng& device, ) :
    //     SearchSession(device) {}
    Exp3SearchSession<size, T> (prng& device, MatrixNode<size, Exp3Stats<size>>* root, T& state, Model<size>& model, double eta) :
        SearchSession(device), root(root), state(state), model(model), eta(eta) {
            if (!root->expanded) {
                T state_ = state;
                expand(root, state_);
            }
        }

    // Infernces the state with the model, and applies that information to the matrix node
    void expand (MatrixNode<size, Exp3Stats<size>>* matrix_node, T& state);

    // Returns the leaf node of a playout. Either game-terminal, pruned-terminal, or just now expanded.
    MatrixNode<size, Exp3Stats<size>>* search (MatrixNode<size, Exp3Stats<size>>* matrix_node_current, T& state);

    //  Public interface for other search. Updates session statistics for this->answer().
    void search (int playouts) {
        this->playouts += playouts;
        for (int playout = 0; playout < playouts; ++ playout) {
            T state_ = state;
            MatrixNode<9 , Exp3Stats<9>>* leaf = this->search(root, state_);

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
    void forecast (std::array<double, size>& forecast0, std::array<double, size>& forecast1, MatrixNode<size, Exp3Stats<size>>* matrix_node);

    // Returned denoised empirical strategies collected over the session (not over the node's history)
    Exp3Answer<size> answer ();
};



// IMPLEMENTATION



template<int size, typename T>
void Exp3SearchSession<size, T> :: expand (MatrixNode<size, Exp3Stats<size>>* matrix_node, T& state) {

    state.actions(matrix_node->pair);

    matrix_node->terminal = (matrix_node->pair.rows*matrix_node->pair.cols == 0);
    
    if (matrix_node->terminal) {
        matrix_node->inference.value_estimate0 = state.payoff;
        matrix_node->inference.value_estimate1 = 1 - state.payoff;
        return;
    }

    matrix_node->inference = model.inference(state);

    matrix_node->expanded = true;
} 

// template <int size>
// MatrixNode<size, Exp3Stats<size>>* Exp3SearchSession<size> :: search (
//     MatrixNode<size, Exp3Stats<size>>* matrix_node_current, 
//     State<size>& state,
//     Model<size>& model) {

//     Exp3SearchSession<<<>>>::search(matrix_node_next, state, model);
// Why is this optional <> ??
// }

template<int size, typename T>
MatrixNode<size, Exp3Stats<size>>* Exp3SearchSession<size, T> :: search (
    MatrixNode<size, Exp3Stats<size>>* matrix_node_current, 
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

            ChanceNode<size, Exp3Stats<size>>* chance_node = matrix_node_current->access(row_idx, col_idx);
            MatrixNode<size, Exp3Stats<size>>* matrix_node_next = chance_node->access(transition_data);
            MatrixNode<size, Exp3Stats<size>>* matrix_node_leaf = search(matrix_node_next, state);

            double u0 = matrix_node_leaf->inference.value_estimate0;
            double u1 = matrix_node_leaf->inference.value_estimate1;
            matrix_node_current->s.gains0[row_idx] += u0 / forecast0[row_idx];
            matrix_node_current->s.gains1[col_idx] += u1 / forecast1[col_idx];
            matrix_node_current->update(u0, u1);
            chance_node->update(u0, u1);
            // matrix_node_current->s.visits0[row_idx] += 1;
            // matrix_node_current->s.visits1[col_idx] += 1;

            return matrix_node_leaf;
        } else {
            expand(matrix_node_current, state);
            return matrix_node_current;
        }
    }
};



template<int size, typename T>
void Exp3SearchSession<size, T> :: forecast (
    std::array<double, size>& forecast0, 
    std::array<double, size>& forecast1, 
    MatrixNode<size, Exp3Stats<size>>* matrix_node) {

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
        double y = std::exp(x * eta / matrix_node->pair.rows);
        forecast0[i] = y;
        sum += y;
    }
    for (int i = 0; i < matrix_node->pair.rows; ++i) {
        forecast0[i] *= (1-eta)/sum;
        forecast0[i] += eta/matrix_node->pair.rows;
    }
    max = 0;
    for (int i = 0; i < matrix_node->pair.cols; ++i) {
        double x = matrix_node->s.gains1[i];
        if (x > max) {
            max = x;
        } 
    }
    sum = 0;
    for (int i = 0; i < matrix_node->pair.cols; ++i) {
        matrix_node->s.gains1[i] -= max;
        double x = matrix_node->s.gains1[i];
        double y = std::exp(x * eta / matrix_node->pair.cols);
        forecast1[i] = y;
        sum += y;
    }
    for (int i = 0; i < matrix_node->pair.cols; ++i) {
        forecast1[i] *= (1-eta)/sum;
        forecast1[i] += eta/matrix_node->pair.cols;
    }
}



template<int size, typename T>
Exp3Answer<size> Exp3SearchSession<size, T> :: answer () {
    Exp3Answer<size> data;
    const int rows = root->pair.rows;
    const int cols = root->pair.cols;
    for (int i = 0; i < rows; ++i) {
        data.strategy0[i] = visits0[i]/ (double)playouts;
    }
    for (int j = 0; j < cols; ++j) {
        data.strategy1[j] = visits1[j]/ (double)playouts;
    }
    const double eta_ = 1/(1 - eta);
    double row_sum = 0;
    for (int i = 0; i < rows; ++i) {
        data.strategy0[i] -= (eta/(double)rows);
        data.strategy0[i] *= data.strategy0[i] > 0 ? eta_ : 0;
        row_sum += data.strategy0[i];
    }
    double col_sum = 0;
    for (int j = 0; j < cols; ++j) {
        data.strategy1[j] -= (eta/(double)cols);
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
    data.value0 = cumulative_value0 / playouts;
    data.value1 = cumulative_value1 / playouts;
    data.playouts = playouts;

    return data;
};