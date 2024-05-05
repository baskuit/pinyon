#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <gmpxx.h>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

/*

Overview and explanation of data structures

BaseData
{
    * 'what position to search'
    * 'what depth to run to'
    * 'what random device, what model to inference'
}

Matrix/Chance Node
{
    The priority here is reducing the size of all of this data as much as possible.
    Anything that can be stored elsewhere will be

    [] = Matrix Node
    O = Chance Node

    []
    | |
    | O ______
    O _______
    |  \  \  \ (seed, quantized prob)
    |  \  \  \
    [] [] [] []

    MatrixNode
    {
        'c-style array to ChanceNodes'
        'last searches action and strategy info for iterative deepning'
    }

    ChanceNode
    {
        'std::unordered_map taking hashed transition obs and storing Branches'
        'vector of pointers to those same branches, ordered by transition probability (for faster value rediscovery)'
        'iterator int for that vector'
        'counter for the number of random transitions tried'
    }

    Branch
    {
        'std::unique_ptr to MatrixNode at the "end of the branch"'
        'random seed to keep State in sync with the tree'
        'quantized prob for branch ordering'
    }
}

TempData
{
    All things related to the algorithms run time

    |DEPTH: 0          |  -> |         | ->  |         |
    |

    'subgame optimistic/pessimistic value matrices'
}

*/

struct Obs {};

uint64_t hash(Obs) {
    return {};
}

struct State {
    Obs obs{};

    void randomize_transition(uint64_t seed) {}
    std::vector<uint8_t> row_actions{}, col_actions{};
    template <typename T>
    void apply_actions(T, T) {}
    bool is_terminal() { return {}; }
    void get_actions() {}
    mpq_class get_payoff() const { return {}; }
    mpq_class get_prob() const { return {}; }
    const Obs &get_obs() const { return obs; }
};

struct Device {
    uint64_t uniform_64() {
        return rand();
    }
};

struct ModelOutput {
    mpq_class value{};
    // std::vector<doulbe> policy{};
};

struct Model {
    ModelOutput inference(State &&state) const {
        return {};
    }
};

struct MatrixNode;

struct BaseData {
    const uint32_t max_depth;
    Device *device;
    const Model *model;
    const State state;

    const uint32_t min_tries;
    const uint32_t max_tries;
    const double max_unexplored;
    const double min_chance_prob;
};

struct HeadData {
    uint32_t min_tries;
    uint32_t max_tries;
    double max_unexplored;
    double min_chance_prob;

    uint32_t depth;

    void step_forward() {
        ++depth;
    }
    void step_back() {
        --depth;
    }
};

struct TempData {
    uint16_t rows;
    uint16_t cols;
    State state;
    mpq_class alpha{0};
    mpq_class beta{1};
    mpq_class min_branch_prob{};

    struct ChanceStats {
        mpq_class alpha_explored{0};
        mpq_class beta_explored{0};
        mpq_class unexplored{1};
    };
    std::vector<ChanceStats> chance_stat_matrix{};

    std::vector<mpq_class> alpha_matrix{};
    std::vector<mpq_class> beta_matrix{};
    std::vector<mpq_class> row_strategy{};
    std::vector<mpq_class> col_strategy{};

    bool must_break{false};

    // void init (const uint16_t rows, const uint16_t cols) {
    //     this->rows = rows;
    //     this->cols = cols;
    //     chance_stat_matrix.resize(rows * cols);
    //     smaller_bounds = false;
    //     new_action = false;s
    // }
};

struct Branch {
    std::unique_ptr<MatrixNode> matrix_node;
    float p;
    uint64_t seed;
    Branch(const double prob, const uint64_t seed) : matrix_node{std::make_unique<MatrixNode>()}, p{static_cast<float>(prob)}, seed{seed} {}
};

struct ChanceNode {
    std::unordered_map<uint64_t, Branch> branches{};
    std::vector<Branch *> sorted_branches{};
    int branch_index{};
    uint32_t tries{};

    Branch *try_get_new_branch(
        const BaseData &base_data,
        const HeadData &head_data,
        const TempData &temp_data,
        TempData &next_temp_data,
        const uint8_t row_idx, const uint8_t col_idx) {

        const auto go = [&](const uint64_t seed) {
            next_temp_data.state = temp_data.state;
            next_temp_data.state.randomize_transition(seed);
            next_temp_data.state.apply_actions(
                next_temp_data.state.row_actions[row_idx],
                next_temp_data.state.col_actions[col_idx]);
        };

        if (branch_index < sorted_branches.size()) {
            go(sorted_branches[branch_index]->seed);
            return sorted_branches[branch_index++];
        }

        auto &chance_data = temp_data.chance_stat_matrix[row_idx * temp_data.cols + col_idx];
        while (
            !(tries > base_data.max_tries ||
              (tries > head_data.min_tries && chance_data.unexplored < head_data.max_unexplored) ||
              chance_data.unexplored <= 0)) {

            const uint64_t seed = base_data.device->uniform_64();
            go(seed);

            Obs obs = next_temp_data.state.get_obs();
            const uint64_t h = hash(obs);
            if (branches.find(h) == branches.end()) {
                branches.try_emplace(h, next_temp_data.state.get_prob().get_d(), seed);
                Branch *new_branch = &branches.at(seed);
                return new_branch;
            }
        }

        return nullptr;
    }

    void reset_stats_for_reexploration() {
        std::sort(sorted_branches.begin(), sorted_branches.end(), [](const Branch *x, const Branch *y) { return x->p > y->p; });
        branch_index = 0;
    }

    void total_reset() {
        branches.clear();
        sorted_branches.clear();
        branch_index = 0;
        tries = 0;
    }
};

// std::vector<ChanceNode> complains about copy contructor, presumably because of std::move
struct ChanceNodeMatrix {
    uint8_t rows;
    uint8_t cols;
    ChanceNode *data;

    void fill(const uint8_t rows, const uint8_t cols) {
        this->rows = rows;
        this->cols = cols;
        data = new ChanceNode[rows * cols];
    }

    ChanceNode &operator()(uint8_t row_idx, uint8_t col_idx) {
        return data[row_idx * cols + col_idx];
    }

    const ChanceNode &operator()(uint8_t row_idx, uint8_t col_idx) const {
        return data[row_idx * cols + col_idx];
    }

    ~ChanceNodeMatrix() {
        if (rows | cols) {
            delete data;
        }
    }
};

struct MatrixNode {

    ChanceNodeMatrix chance_node_matrix;

    struct ActionProb {
        uint8_t index;
        uint8_t discrete_prob{};

        ActionProb(const uint8_t index) : index{index} {}

        operator uint8_t() const {
            return index;
        }
        bool operator<(const ActionProb &a) const {
            return discrete_prob < a.discrete_prob;
        }
    };

    struct Solution {
        std::vector<ActionProb> action_indices{};
        uint8_t boundary{0};

        Solution() {}

        Solution(uint8_t size) {
            for (uint8_t i = 0; i < size; ++i) {
                action_indices.push_back(i);
            }
        }

        void add_index(uint8_t index) {
            assert(boundary >= 0 && boundary < action_indices.size() - 1);
            std::swap(action_indices[index], action_indices[boundary++]);
        }

        void remove_index(uint8_t index) {
            assert(boundary >= 1 && boundary < action_indices.size());
            std::swap(action_indices[index], action_indices[--boundary]);
        }
    };
    Solution I;
    Solution J;

    void prune() {
        for (uint8_t i = I.boundary; i < chance_node_matrix.rows; ++i) {
            for (uint8_t j = J.boundary; j < chance_node_matrix.cols; ++j) {
                auto &chance_node = chance_node_matrix(I.action_indices[i], J.action_indices[j]);
                chance_node.total_reset();
            }
        }
    }
};

struct Search {

    void update_chance_stats(
        TempData::ChanceStats &stats,
        MatrixNode *next_matrix_node,
        BaseData &base_data,
        HeadData &head_data,
        TempData &temp_data,
        TempData &next_temp_data) {

        const mpq_class prob = next_temp_data.state.get_prob();
        stats.unexplored -= prob;
        mpq_class alpha;
        mpq_class beta;
        if (next_temp_data.state.is_terminal()) {
            alpha = beta = next_temp_data.state.get_payoff();
        } else if (head_data.min_chance_prob > prob) { // TODO init min_chance_prob
            alpha = 0;
            beta = 1;
        } else if (head_data.depth + 1 == base_data.max_depth) {
            const ModelOutput output = base_data.model->inference(std::move(next_temp_data.state));
            alpha = beta = output.value;
        } else {
            const auto [a, b] = this->alpha_beta(next_matrix_node, base_data, head_data, next_temp_data);
            alpha = a;
            beta = b;
        }
        stats.alpha_explored -= alpha * prob;
        stats.beta_explored -= beta * prob;
    }

    void solve_chance_node(
        MatrixNode *matrix_node,
        BaseData &base_data,
        HeadData &head_data,
        TempData &temp_data,
        TempData &next_temp_data,
        const uint8_t row_idx, const uint8_t col_idx) {

        auto &chance_data = temp_data.chance_stat_matrix[0];
        Branch *new_branch;
        ChanceNode &chance_node = matrix_node->chance_node_matrix(row_idx, col_idx);
        while (new_branch = chance_node.try_get_new_branch(base_data, head_data, temp_data, next_temp_data, row_idx, col_idx)) {
            update_chance_stats(chance_data, new_branch->matrix_node.get(), base_data, head_data, temp_data, next_temp_data);
        }
    }

    std::pair<mpq_class, mpq_class>
    solve_subgame(
        MatrixNode *matrix_node,
        BaseData &base_data,
        HeadData &head_data,
        TempData &temp_data,
        TempData &next_temp_data) {

        return {};
    }

    void initialize_submatrix(
        MatrixNode *matrix_node,
        BaseData &base_data,
        HeadData &head_data,
        TempData &temp_data,
        TempData &next_temp_data) {

        if (!matrix_node->chance_node_matrix.rows) {
            matrix_node->chance_node_matrix.fill(temp_data.state.row_actions.size(), temp_data.state.col_actions.size());
        }

        // prune here possibly

        for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
            const uint8_t row_idx = matrix_node->I.action_indices[i].index;
            for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                const uint8_t col_idx = matrix_node->J.action_indices[j].index;
                solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data, row_idx, col_idx);
            }
        }
    }

    // pass a reference to best score
    void row_best_response(mpq_class &alpha, MatrixNode *matrix_node, TempData &temp_data) {
        struct BestResponse {
            struct Data {
            };
            std::vector<Data> data_vector{};
        };

        for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
        }

        for (uint8_t i = matrix_node->I.boundary; i < matrix_node->chance_node_matrix.rows; ++i) {
        }

        temp_data.must_break = false;
    }

    void col_best_response(mpq_class &beta, MatrixNode *matrix_node, TempData &temp_data) {
        struct BestResponse {
        };
    }

    std::pair<mpq_class, mpq_class>
    alpha_beta(
        MatrixNode *matrix_node,
        BaseData &base_data,
        HeadData &head_data,
        TempData &temp_data) {

        head_data.step_forward();

        temp_data.state.get_actions();

        // allocate for all next function calls
        TempData next_temp_data;
        // copy over state or w/e

        this->initialize_submatrix(matrix_node, base_data, head_data, temp_data, next_temp_data);
        // if new node, expand. otherwise populate the old NE sub matrix

        while (temp_data.alpha < temp_data.beta && !temp_data.must_break) {

            auto [a0, b0] = this->solve_subgame(matrix_node, base_data, head_data, temp_data, next_temp_data);

            this->row_best_response(a0, matrix_node, temp_data);
            this->col_best_response(b0, matrix_node, temp_data);

            temp_data.alpha = std::min(b0, temp_data.alpha);
            temp_data.beta = std::min(a0, temp_data.beta);
        }

        for (uint8_t i = 0; i < matrix_node->chance_node_matrix.rows * matrix_node->chance_node_matrix.cols; ++i) {
            matrix_node->chance_node_matrix.data[i].reset_stats_for_reexploration();
        }

        head_data.step_back();

        return {temp_data.alpha, temp_data.beta};
    }
};

int main() {
    srand(time(NULL));

    MatrixNode node{};
    return 0;
}