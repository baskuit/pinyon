#include <set>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>

struct MatrixNode;

struct Branch {
    std::unique_ptr<MatrixNode> matrix_node;
    double prob;
    uint64_t seed;
};

struct ChanceNode {
    std::unordered_map<uint64_t, Branch> branches{};
    bool has_preexisting_branches{false};
    int preexisting_branch_index{};


    
};

struct MatrixNode {
    int rows;
    int cols;
    std::vector<ChanceNode> chance_node_matrix{};

    void fill (int rows, int cols) {
        this->rows = rows;
        this->cols = cols;
        this->chance_node_matrix.resize(rows * cols);
    }

    void prune (const std::vector<int> &I, const std::vector<int> &J) {
        for (int i = 0; i < rows; ++i) {
            if (std::find(I.begin(), I.end(), i) != I.end()) {
                continue;
            }
            for (int j = 0; j < cols; ++j) {
                if (std::find(J.begin(), J.end(), j) != J.end()) {
                    continue;
                }
                auto &chance_node = chance_node_matrix[i * cols + j];
                chance_node.branches.clear();
                chance_node.has_preexisting_branches = false;
                chance_node.preexisting_branch_index = 0;
            }
        }
    }
};


int main () {
    MatrixNode node{};
    node.fill(2, 2);
    for (const auto &chance_node : node.chance_node_matrix) {
        std::cout << "num branches: " << chance_node.branches.size() << std::endl;
        std::cout << "pre existing branches: " << chance_node.has_preexisting_branches << std::endl;
        std::cout << "index: " << chance_node.preexisting_branch_index << std::endl;
        std::cout << std::endl;
    }
    std::vector<int> I{0};
    std::vector<int> J{0};
    node.prune(I, J);

    return 0;
}