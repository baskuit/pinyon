#pragma once

#include <tree/tree.hh>

#include <array>

/*

Cache friendly subtree

*/

template <class Algorithm, size_t layers, int matrix_branch_factor, int chance_branch_factor>
struct Stalk {

    static constexpr size_t matrix_node_size = sizeof(MatrixNode<Algorithm>);
    static constexpr size_t chance_node_size = sizeof(ChanceNode<Algorithm>);
    static constexpr size_t n_matrix_nodes = 0;
    static constexpr size_t n_chance_nodes = 0;
    // TODO arithmetic

    std::array<MatrixNode<Algorithm>, n_matrix_nodes> matrix_nodes;
    std::array<ChanceNode<Algorithm>, n_chance_nodes> chance_nodes;


    Stalk (MatrixNode<Algorithm> &root) {
        SortMove(&root, layers);
    }

    void SortMove(MatrixNode<Algorithm> *matrix_node, int depth_bound, size_t current_index = 0,) {
        // Move hottest subtree into struct
    }
    void SortMove(ChanceNode<Algorithm> *chance_node, int depth_bound, size_t current_index = 0) {
        // Move hottest subtree into struct
    }
};