#include <set>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <gmpxx.h>

struct MatrixNode;

struct Branch
{
    std::unique_ptr<MatrixNode> matrix_node;
    double prob;
    uint64_t seed;

    Branch(const double prob, const uint64_t seed) : matrix_node{std::make_unique<MatrixNode>()}, prob{prob}, seed{seed} {}
};

struct HeadData
{
    uint32_t min_tries;
    uint32_t max_tries;
    double max_unexplored;
    uint32_t depth;
};

struct ChanceNode
{
    std::unordered_map<uint64_t, Branch> branches{};
    bool has_preexisting_branches{false};
    int preexisting_branch_index{};
    uint32_t tries{};

    mpq_class alpha_explored{};
    mpq_class beta_explored{};
    mpq_class unexplored{};

    Branch *try_get_new_branch(const HeadData &head_data)
    {
        if (
            tries > head_data.max_tries ||
                (tries > head_data.min_tries && unexplored < head_data.max_unexplored);)
        {
            return nullptr;
        }
    }

    void reset()
    {
        alpha_explored = beta_explored = 0;
        unexplored = 1;
    }
};

struct ChanceNodeMatrix
{
    uint8_t rows;
    uint8_t cols;
    ChanceNode *data;

    void fill(const uint8_t rows, const uint8_t cols)
    {
        this->rows = rows;
        this->cols = cols;
        data = new ChanceNode[rows * cols];
    }

    ChanceNode &operator()(uint8_t row_idx, uint8_t col_idx)
    {
        return data[row_idx * cols + col_idx];
    }

    const ChanceNode &operator()(uint8_t row_idx, uint8_t col_idx) const
    {
        return data[row_idx * cols + col_idx];
    }

    ~ChanceNodeMatrix()
    {
        if (rows | cols)
        {
            delete data;
        }
    }
};

struct MatrixNode
{

    ChanceNodeMatrix chance_node_matrix{};

    void fill(uint8_t rows, uint8_t cols)
    {
        this->chance_node_matrix.fill(rows, cols);
    }

    // void prune (const std::vector<int> &I, const std::vector<int> &J) {
    //     for (int i = 0; i < rows; ++i) {
    //         if (std::find(I.begin(), I.end(), i) != I.end()) {
    //             continue;
    //         }
    //         for (int j = 0; j < cols; ++j) {
    //             if (std::find(J.begin(), J.end(), j) != J.end()) {
    //                 continue;
    //             }
    //             auto &chance_node = chance_node_matrix[i * cols + j];
    //             chance_node.branches.clear();
    //             chance_node.has_preexisting_branches = false;
    //             chance_node.preexisting_branch_index = 0;
    //         }
    //     }
    // }
};

int main()
{
    MatrixNode node{};
    // node.fill(2, 2);
    // std::unordered_map<uint64_t, Branch> branches{};
    // auto &&b = std::move(branches);

    // for (const auto &chance_node : node.chance_node_matrix) {
    //     std::cout << "num branches: " << chance_node.branches.size() << std::endl;
    //     std::cout << "pre existing branches: " << chance_node.has_preexisting_branches << std::endl;
    //     std::cout << "index: " << chance_node.preexisting_branch_index << std::endl;
    //     std::cout << std::endl;
    // }
    // std::vector<int> I{0};
    // std::vector<int> J{0};
    // node.prune(I, J);
    return 0;
}