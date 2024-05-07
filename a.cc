#include "a.h"

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
    uint16_t rows, cols;
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
    bool new_row_action{false};
    bool new_col_action{false};

    void reset_flags() {
        must_break = true;
        new_row_action = false;
        new_col_action = false;
    }
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

    void total_reset() {
        branches.clear();
        sorted_branches.clear();
        branch_index = 0;
        tries = 0;
    }
};

// std::vector<ChanceNode> complains about copy contructor, presumably because of std::move
struct ChanceNodeMatrix {
    uint8_t rows, cols;
    ChanceNode *data;

    void init(const uint8_t rows, const uint8_t cols) {
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
        uint8_t idx;
        uint8_t discrete_prob{};

        ActionProb(const uint8_t index) : idx{index} {}

        operator uint8_t() const {
            return idx;
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
        MatrixNode *next_matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) {

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
        MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data,
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
        MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) {

        return {};
    }

    void initialize_submatrix(
        MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) {

        if (!matrix_node->chance_node_matrix.rows) {
            matrix_node->chance_node_matrix.init(temp_data.state.row_actions.size(), temp_data.state.col_actions.size());
        }

        // prune here possibly

        for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
            const uint8_t row_idx = matrix_node->I.action_indices[i].idx;
            for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                const uint8_t col_idx = matrix_node->J.action_indices[j].idx;
                solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data, row_idx, col_idx);
            }
        }
    }

    // pass a reference to best score
    void row_best_response(
        mpq_class &alpha,
        MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) {

        struct BestResponse {
            uint8_t row_idx, col_idx;
            mpq_class value{};
            std::vector<mpq_class> priority_vector{};
        };

        uint8_t new_action_index;

        for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
        }

        // actions not in I
        for (uint8_t i = matrix_node->I.boundary; i < matrix_node->chance_node_matrix.rows; ++i) {
            BestResponse data{};
            data.row_idx = matrix_node->I.action_indices[i].idx;
            data.priority_vector.resize(matrix_node->J.boundary);

            // set up priority, score vector
            {
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    if (matrix_node->J.action_indices[j].discrete_prob == 0) {
                        continue;
                    }
                    data.col_idx = matrix_node->J.action_indices[j].idx;
                    uint8_t chance_stat_index = data.row_idx * temp_data.cols + data.col_idx;

                    data.priority_vector[j] =
                        temp_data.chance_stat_matrix[chance_stat_index].unexplored *
                        temp_data.col_strategy[j];
                }
            }

            // returns 0x1111111 once the support chance nodes are full explored
            const auto get_max_priority_index = [&]() {
                uint8_t max_index = 0xFF;
                mpq_class max_prio{-1};
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    if (data.priority_vector[j] > std::max(mpq_class{}, max_prio)) {
                        max_index = j;
                    }
                }
                return max_index;
            };

            // explore as much as possible
            for (uint8_t j; (j = get_max_priority_index()) != 0xFF;) {
                ChanceNode &chance_node = matrix_node->chance_node_matrix(data.row_idx, j);
                Branch *branch = chance_node.try_get_new_branch(base_data, head_data, temp_data, next_temp_data, data.row_idx, data.col_idx);
                if (branch == nullptr) {
                    data.priority_vector[j] = 0;
                } else [[likely]] {
                    const mpq_class prob = next_temp_data.state.get_prob();
                    const uint8_t entry_idx = data.row_idx * temp_data.cols + data.col_idx;
                    update_chance_stats(temp_data.chance_stat_matrix[entry_idx], branch->matrix_node.get(), base_data, head_data, temp_data, next_temp_data);
                    data.priority_vector[j] -= prob * temp_data.col_strategy[data.col_idx]; // TODO maybe add prob to temp_data
                    data.value += next_temp_data.beta * prob * temp_data.col_strategy[data.col_idx];
                }
            }

            for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                if (matrix_node->J.action_indices[j].discrete_prob == 0) {
                    continue;
                }
                const uint8_t entry_idx = data.row_idx * temp_data.cols + data.col_idx;
                data.value += temp_data.chance_stat_matrix[entry_idx].beta_explored * temp_data.col_strategy[matrix_node->J.action_indices[j].idx];
            }
        }
    }

    void col_best_response(
        mpq_class &alpha,
        MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) {

        struct BestResponse {
        };
    }

    void sort_branches_and_reset(MatrixNode *matrix_node) const {
        for (uint8_t i = 0; i < matrix_node->chance_node_matrix.rows * matrix_node->chance_node_matrix.cols; ++i) {
            // chance_data.sort_branches_and_reset_index();
            auto &chance_data = matrix_node->chance_node_matrix.data[i];
            // because the branches are added by sampling, this array should be mostly sorted on its own
            // idk if std::sort takes full advantage of this, and that only a tail of the array is unsorted;
            // TODO
            std::sort(
                chance_data.sorted_branches.begin(),
                chance_data.sorted_branches.end(),
                [](const Branch *x, const Branch *y) { return x->p > y->p; });
            chance_data.branch_index = 0;
        }
    }

    std::pair<mpq_class, mpq_class>
    alpha_beta(
        MatrixNode *matrix_node,
        BaseData &base_data,
        HeadData &head_data,
        TempData &temp_data) {

        head_data.step_forward();

        temp_data.state.get_actions();

        // only allocate for all next function calls, modify in other functions when necessary
        TempData next_temp_data;

        this->initialize_submatrix(matrix_node, base_data, head_data, temp_data, next_temp_data);
        // if new node, expand. otherwise populate the old NE sub matrix

        while (temp_data.alpha < temp_data.beta && !temp_data.must_break) {

            auto [a0, b0] = this->solve_subgame(matrix_node, base_data, head_data, temp_data, next_temp_data);

            this->row_best_response(a0, matrix_node, base_data, head_data, temp_data, next_temp_data);
            this->col_best_response(b0, matrix_node, base_data, head_data, temp_data, next_temp_data);

            temp_data.alpha = std::min(b0, temp_data.alpha);
            temp_data.beta = std::min(a0, temp_data.beta);
        }

        this->sort_branches_and_reset(matrix_node);

        head_data.step_back();

        return {temp_data.alpha, temp_data.beta};
    }
};

int main() {
    srand(time(NULL));

    MatrixNode node{};
    return 0;
}