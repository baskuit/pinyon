#pragma once
#include <vector>

template <typename _Action>
struct Actions {
    uint8_t rows, cols;
    std::vector<_Action> row_col_actions;
};

template <typename _Action>
struct ActionsA {
    uint8_t rows, cols;
    std::vector<_Action> row_actions, col_actions;
};