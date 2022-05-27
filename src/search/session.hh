#pragma once

#include "../tree/tree_linked_list.hh"
#include "../state/state.hh"
#include "../model/model.hh"

struct SearchSessionData {

    int playouts;
    int rows;
    int cols;
    int* visits0;
    int* visits1;
    float cumulative_score0;
    float cumulative_score1;
    float* nash_solution0;
    float* nash_solution1;

    SearchSessionData () {
        playouts = 0;
        rows = 0;
        cols = 0;
        visits0 = nullptr;
        visits1 = nullptr;
        cumulative_score0 = 0;
        cumulative_score1 = 0;
        nash_solution0 = nullptr;
        nash_solution1 = nullptr;
    };

    SearchSessionData (State& state) {
        playouts = 0;
        rows = state.rows;
        cols = state.cols;
        visits0 = nullptr;
        visits1 = nullptr;
        cumulative_score0 = 0;
        cumulative_score1 = 0;
        nash_solution0 = nullptr;
        nash_solution1 = nullptr;
    };

    ~SearchSessionData () {
        delete visits0;
        delete visits1;
        delete nash_solution0;
        delete nash_solution1;
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
        result.nash_solution0 = new float[rows_min];
        result.nash_solution1 = new float[cols_min];
        for (int row_idx = 0; row_idx < rows_min; ++row_idx) {
            result.visits0[row_idx] = visits0[row_idx] + S.visits0[row_idx];
            result.nash_solution0[row_idx] = portionA * nash_solution0[row_idx] + portionB * S.nash_solution0[row_idx];

        }
        for (int col_idx = 0; col_idx < cols_min; ++col_idx) {
            result.visits1[col_idx] = visits1[col_idx] + S.visits1[col_idx];
            result.nash_solution1[col_idx] = portionA * nash_solution1[col_idx] + portionB * S.nash_solution1[col_idx];
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

    int* visits0;
    int* visits1;
    int playouts;

private:
    virtual MatrixNode* search (MatrixNode* matrix_node_current, State* state){
        return nullptr;
    };

};