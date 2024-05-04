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
Data is stored in 3 places

- Tree (Persists between searches)
- Root (i.e. initial search params)
- Head (info that is allocated once and inc/dec as we explore)
- Body (initialized when solving a matrix node)
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
    mpq_class get_payoff () const {return {};}
    mpq_class get_prob () const {return {};}
    const Obs& get_obs () const {return obs;}
};

struct Device {
    uint64_t uniform_64() {
        return rand();
    }
};

struct Model {};

struct MatrixNode;

struct RootData {
    const uint32_t max_depth;
    Device *device;
    Model *model;
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

struct BodyData {
    uint8_t rows;
    uint8_t cols;
    State state;
    mpq_class alpha{0};
    mpq_class beta{1};

    struct ChanceStats {
        mpq_class alpha_explored{0};
        mpq_class beta_explored{0};
        mpq_class unexplored{1};
    };
    std::vector<ChanceStats> chance_stat_matrix{};

    std::vector<mpq_class> alpha_matrix;
    std::vector<mpq_class> beta_matrix;
    std::vector<mpq_class> row_strategy;
    std::vector<mpq_class> col_strategy;

    bool smaller_bounds;
    bool new_action;
};

struct Branch {
    std::unique_ptr<MatrixNode> matrix_node;
    double prob;
    uint64_t seed;

    Branch(const double prob, const uint64_t seed) : matrix_node{std::make_unique<MatrixNode>()}, prob{prob}, seed{seed} {}
};

struct ChanceNode {
    std::unordered_map<uint64_t, Branch> branches{};
    std::vector<Branch *> sorted_branches{};
    int branch_index{};
    uint32_t tries{};

    Branch *try_get_new_branch(
        const RootData &root_data,
        const HeadData &head_data,
        const BodyData &body_data,
        BodyData &new_body_data,
        const uint8_t row_idx, const uint8_t col_idx) {

        if (branch_index < sorted_branches.size()) {
            new_body_data.state = body_data.state;
            new_body_data.state.randomize_transition(sorted_branches[branch_index]->seed);
            new_body_data.state.apply_actions(
                new_body_data.state.row_actions[row_idx],
                new_body_data.state.col_actions[col_idx]);
            return sorted_branches[branch_index++];
        }
        auto &chance_data = body_data.chance_stat_matrix[row_idx * 4 + col_idx];
        while (
            tries <= root_data.max_tries &&
            (tries <= head_data.min_tries || chance_data.unexplored >= head_data.max_unexplored) &&
            chance_data.unexplored > 0) {

            new_body_data.state = body_data.state;
            const uint64_t seed = root_data.device->uniform_64();
            new_body_data.state.randomize_transition(seed);
            new_body_data.state.apply_actions(
                new_body_data.state.row_actions[row_idx],
                new_body_data.state.col_actions[col_idx]);

            Obs obs = new_body_data.state.get_obs();
            uint64_t h = hash(obs);
            if (branches.find(h) == branches.end()) {
                branches.try_emplace(h, new_body_data.state.get_prob().get_d(), seed);
                Branch *new_branch = &branches.at(seed);
                return new_branch;
            }
        }

        return nullptr;
    }

    void reset_stats_for_reexploration() {
        std::sort(sorted_branches.begin(), sorted_branches.end(), [](const Branch *x, const Branch *y) { return x->prob > y->prob; });
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

    template <typename ActionData = ActionProb>
    struct ActionSet {
        std::vector<ActionData> action_indices{};
        uint8_t boundary{0};

        ActionSet() {}

        ActionSet(uint8_t size) {
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
    ActionSet<> I;
    ActionSet<> J;

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
    BodyData initial_data;

    void solve_chance_node(
        MatrixNode *matrix_node,
        const RootData &root_data,
        HeadData &head_data,
        BodyData &body_data,
        BodyData &new_body_data,
        const uint8_t row_idx, const uint8_t col_idx) {

        ChanceNode &chance_node = matrix_node->chance_node_matrix(row_idx, col_idx);
        auto &chance_data = body_data.chance_stat_matrix[0];
        Branch *new_branch;
        while (new_branch = chance_node.try_get_new_branch(root_data, head_data, body_data, new_body_data, row_idx, col_idx)) {
            if (new_body_data.state.is_terminal()) {
                mpq_class prob;
                chance_data.alpha_explored -= new_body_data.state.get_payoff() * prob;
                chance_data.beta_explored -= new_body_data.state.get_payoff() * prob;
                chance_data.unexplored -= prob;
            }
            if (head_data.depth + 1 == root_data.max_depth) {
                mpq_class prob;
                // root_data.model->get_inference(std::move(new_body_data.state))
                // chance_data.alpha_explored -= state.get_payoff * prob;
                // chance_data.beta_explored -= state.get_payoff * prob;
                chance_data.unexplored -= prob;
            }
            auto [a, b] = this->alpha_beta(matrix_node, root_data, head_data, body_data);
            chance_data.unexplored -= new_branch->prob;
            chance_data.unexplored.canonicalize();
        }
    }

    void initialize_submatrix(
        MatrixNode *matrix_node,
        const RootData &root_data,
        HeadData &head_data,
        BodyData &body_data,
        BodyData &new_body_data) {

        for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
            const uint8_t row_idx = matrix_node->I.action_indices[i].index;
            for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                const uint8_t col_idx = matrix_node->J.action_indices[j].index;
                solve_chance_node(matrix_node, root_data, head_data, body_data, new_body_data, row_idx, col_idx);
            }
        }
    }

    // pass a reference to best score
    void row_best_response(mpq_class &alpha, MatrixNode *matrix_node, BodyData &body_data) {
        struct BestResponse {
            struct Data {
            };
            std::vector<Data> data_vector{};
        };

        for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
        }

        for (uint8_t i = matrix_node->I.boundary; i < matrix_node->chance_node_matrix.rows; ++i) {
        }
    }

    void col_best_response(mpq_class &beta, MatrixNode *matrix_node, BodyData &body_data) {
        struct BestResponse {
        };
    }

    std::pair<mpq_class, mpq_class> solve_subgame() { return {{}, {}}; }

    std::pair<mpq_class, mpq_class>
    alpha_beta(
        MatrixNode *matrix_node,
        const RootData &root_data,
        HeadData &head_data,
        BodyData &body_data) {

        head_data.step_forward();

        body_data.state.get_actions();

        // allocate for all next function calls
        BodyData next_body_data;
        // copy over state or w/e

        this->initialize_submatrix(matrix_node, root_data, head_data, body_data, next_body_data);
        // if new node, expand. otherwise populate the old NE sub matrix

        while (body_data.alpha < body_data.beta) {

            auto [a0, b0] = this->solve_subgame();

            this->row_best_response(a0, matrix_node, body_data);
            this->col_best_response(b0, matrix_node, body_data);

            body_data.alpha = std::min(b0, body_data.alpha);
            body_data.beta = std::min(a0, body_data.beta);
        }

        for (uint8_t i = 0; i < matrix_node->chance_node_matrix.rows * matrix_node->chance_node_matrix.cols; ++i) {
            matrix_node->chance_node_matrix.data[i].reset_stats_for_reexploration();
        }

        head_data.step_back();

        return {body_data.alpha, body_data.beta};
    }
};

int main() {
    srand(time(NULL));

    MatrixNode node{};
    return 0;
}