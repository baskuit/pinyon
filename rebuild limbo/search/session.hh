#pragma once

#include <vector>

#include "../tree/tree_linked_list.hh"
#include "../state/state.hh"
#include "../model/model.hh"

struct SearchSessionData {

    int playouts;
    int rows;
int cols;
    std::vector<int> visits0;
    std::vector<int> visits1;
    std::vector<float> strategy0;
    std::vector<float> strategy1;
    float cumulative_score0;
    float cumulative_score1;

    SearchSessionData () {
        playouts = 0;
        rows = 0;
        cols = 0;
        cumulative_score1 = 0;
    };

    SearchSessionData (State& state) {
        PairActions pair = state.actions();
        playouts = 0;
        rows = pair.rows;
        cols = pair.cols;
        cumulative_score0 = 0;
        cumulative_score1 = 0;
    };

    ~SearchSessionData () {
        delete &visits0;
        delete &visits1;
        delete &strategy0;
        delete &strategy1;
    }

    // 'Add' the results of two searches. New solved strategies are the weighted sums.
    SearchSessionData operator+ (SearchSessionData S) {
        SearchSessionData result;
        result.playouts = playouts + S.playouts;
        const float portionA = playouts / result.playouts;
        const float portionB = S.playouts / result.playouts;

        const int rows_min = rows < S.rows ? rows : S.rows;
        int cols_min = cols < S.cols ? cols : S.cols;
        result.visits0 = new int[rows_min];
        result.visits1 = new int[cols_min];
        result.strategy0 = new float[rows_min];
        result.strategy1 = new float[cols_min];
        for (int row_idx = 0; row_idx < rows_min; ++row_idx) {
            result.visits0[row_idx] = visits0[row_idx] + S.visits0[row_idx];
            result.strategy0[row_idx] = portionA * strategy0[row_idx] + portionB * S.strategy0[row_idx];

        }
        for (int col_idx = 0; col_idx < cols_min; ++col_idx) {
            result.visits1[col_idx] = visits1[col_idx] + S.visits1[col_idx];
            result.strategy1[col_idx] = portionA * strategy1[col_idx] + portionB * S.strategy1[col_idx];
        }
        result.cumulative_score0 = cumulative_score0 + S.cumulative_score0;
        result.cumulative_score1 = cumulative_score1 + S.cumulative_score1;
        return result;
    }
};

class SearchSession {
public:
    MatrixNode* root;
    State* state;
    Model* model;
    SearchSession() {};
    SearchSession (MatrixNode* root, State* state, Model* model, int* visits0, int* visits1) :
    root(root), state(state), model(model) {};

    virtual void search (int playouts) {};
    virtual SearchSessionData answer () {
        return SearchSessionData();
    }

    int playouts;
    int* visits0;
    int* visits1;
    float cumulativeScore0;
    float cumulativeScore1;

    virtual MatrixNode* search (MatrixNode* matrix_node_current, State* state){
        return nullptr;
    };

};