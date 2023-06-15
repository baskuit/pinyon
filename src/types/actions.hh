#pragma once
#include <vector>

// TODO currently unused. But there is some potential for node size savings by using one vector for all this information

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