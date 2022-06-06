#pragma once

#include "../tree/node.hh"
#include "../state/state.hh"
#include "../model/model.hh"

struct SearchSessionData {

    int playouts = 0;
    int rows = 0;
    int cols = 0;
    float cumulative_score0 = 0.f;
    float cumulative_score1 = 0.f;
    int* visits0 = nullptr;
    int* visits1 = nullptr;
    float* strategy0 = nullptr;
    float* strategy1 = nullptr;

    SearchSessionData (const PairActions* pair) : 
    rows(pair->rows), cols(pair->cols), 
    visits0(new int[pair->rows] {0}), visits1(new int[pair->cols] {0}), 
    strategy0(new float[pair->rows] {0.f}), strategy1(new float[pair->cols] {0.f})  {};
    SearchSessionData (int rows, int cols) : 
    rows(rows), cols(cols), 
    visits0(new int[rows] {0}), visits1(new int[cols] {0}), 
    strategy0(new float[rows] {0.f}), strategy1(new float[cols] {0.f})  {};

    ~SearchSessionData () {
        delete visits0;
        delete visits1;
        delete strategy0;
        delete strategy1;
    }

    // 'Add' the results of two searches. New solved strategies are the weighted sums.
    SearchSessionData operator+ (SearchSessionData S) {
        const int rows_min = rows < S.rows ? rows : S.rows;
        const int cols_min = cols < S.cols ? cols : S.cols;
        SearchSessionData result(rows_min, cols_min);
        result.playouts = playouts + S.playouts;
        const float portionA =   playouts / result.playouts;
        const float portionB = S.playouts / result.playouts;

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
    SearchSession (MatrixNode* root, State* state, Model* model) :
    root(root), state(state), model(model), playouts(0) {
    };

    virtual void search (int playouts) {};
    virtual SearchSessionData answer () {
        return SearchSessionData(root->rows, root->cols);
    }

    // This is tracked in the node, but multiple search sessions may operate on the same node/tree
    int playouts = 0;
    int* visits0 = nullptr;
    int* visits1 = nullptr;

    virtual MatrixNode* search (MatrixNode* matrix_node_current, State* state){
        return nullptr;
    };
    virtual void expand (MatrixNode* matrix_node, State* state, Model* model) {
        return;
    };

};