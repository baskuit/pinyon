#pragma once

#include <string.h>
#include <iostream>

#include "../libsurskit/math.hh"
#include "state.hh"

PairActions :: ~PairActions () {
    delete [] actions0;
    delete [] actions1;
}

float State :: rollout () {
    PairActions pair = this->actions();
    while (pair.rows * pair.cols != 0) {
        int row_idx = this->device.random_int(pair.rows);
        int col_idx = this->device.random_int(pair.cols);
        this->transition(pair.actions0[], col_idx);
    }
    return this->info->payoff;
}

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

SolvedState :: ~SolvedState () {
    delete [] strategy0;
    delete [] strategy1;
};

    // State

SolvedState* SolvedState :: copy () {
    SolvedState* x = new SolvedState(rows, cols, payoff);
    for (int i = 0; i < rows; ++i) {
        x->strategy0[i] = strategy0[i];
    }
    for (int j = 0; j < cols; ++j) {
        x->strategy1[j] = strategy1[j];
    }
    return x;
}