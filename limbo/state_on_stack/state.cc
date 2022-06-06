#pragma once

#include <string.h>

#include "../libsurskit/math.hh"
#include "state.hh"

    // shallow copy will not work since we modify the strategies during transition
SolvedState :: SolvedState (SolvedState const& info) {
    terminal = info.terminal;
    rows = info.rows;
    cols = info.cols;
    payoff = info.payoff;
    strategy0 = new float[rows];
    strategy1 = new float[cols];
    memcpy(strategy0, info.strategy0, rows*sizeof(float)); 
    memcpy(strategy1, info.strategy1, cols*sizeof(float));
};

    // State

float SolvedState :: rollout () {
    PairActions pair = this->actions();
    while (pair->rows * pair->cols != 0) {
        int row_idx = this->device.random_int(pair->rows);
        int col_idx = this->device.random_int(pair->cols);
        this->transition(pair->actions0[row_idx], pair->actions1[col_idx]);
    }
    return this->payoff;
}