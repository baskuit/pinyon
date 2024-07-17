#pragma once

#include "lib.h"

#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

/*

TODO:
use std::vector for chance_nodes - maybe just need noexcept?
use array of temp_data that's allocated once in the run() call
try using a/b from subgame solution to warm start best response
*/

template <typename T>
constexpr bool fuzzy_equals(const T a, const T b) {
    static constexpr float eps = .0001;
    return a < (b + eps) && (a + eps) > b;
}

bool fuzzy_equals(mpq_class &a, mpq_class &b) {
    a.canonicalize();
    b.canonicalize();
    return a == b;
}

static int indent = 0;

template <typename Types, bool debug = false>
struct AlphaBetaRefactor : Types {

    static void mpq_vector_print(const std::vector<float> &input) {
        if constexpr (debug) {
            for (int i = 0; i < input.size(); ++i) {
                std::cout << input[i] << ", ";
            }
            std::cout << std::endl;
        }
    }

    template <typename... T>
    static void debug_print(T... args) {
        if constexpr (debug) {
            for (int i = 0; i < indent; ++i) {
                std::cout << " ";
            }
            ((std::cout << args), ...);
        }
    }

    template <typename... T>
    static void debug_print_no_indent(T... args) {
        if constexpr (debug) {
            ((std::cout << args), ...);
        }
    }

    using PRNG = typename Types::PRNG;
    using State = typename Types::State;
    using Obs = typename Types::Obs;
    using Model = typename Types::Model;
    using ModelOutput = typename Types::ModelOutput;

    static uint64_t hash(const Obs &obs) {
        const typename Types::ObsHash hasher{};
        return hasher(obs);
        // return static_cast<uint64_t>(obs);
    }

    struct MatrixNode;

    struct BaseData {
        uint32_t max_depth;
        PRNG *device;
        Model *model;
        uint32_t min_tries;
        std::vector<uint32_t> max_tries;
        float max_unexplored;
        float min_reach_prob;
        // output stats
        size_t matrix_node_count = 0;
        size_t alpha_beta_count = 0;
        size_t terminal_count = 0;
        size_t inference_count = 0;
        size_t total_inference_time = 0;
        size_t total_solves[9][9]{};
        size_t total_solves_raw{};

        BaseData(
            uint32_t max_depth,
            PRNG *device,
            Model *model,
            uint32_t min_tries,
            std::vector<uint32_t> max_tries,
            float max_unexplored,
            float min_reach_prob)
            : max_depth{max_depth}, device{device}, model{model}, min_tries{min_tries}, max_tries{max_tries}, max_unexplored{max_unexplored}, min_reach_prob{min_reach_prob} {}
    };

    struct HeadData {
        uint32_t min_tries;
        uint32_t max_tries;
        uint32_t depth{0};

        HeadData(
            uint32_t min_tries,
            uint32_t max_tries) : min_tries{min_tries}, max_tries{max_tries} {}

        void step_forward() {
            ++depth;
        }
        void step_back() {
            --depth;
        }
    };

    struct TempData {
        State state{};
        uint16_t rows, cols;
        float alpha{0};
        float beta{1};
        float chance_reach_prob{1};

        struct ChanceStats {
            float alpha_explored{0};
            float beta_explored{0};
            float unexplored{1};
        };

        std::vector<ChanceStats> chance_stat_matrix{};

        std::vector<int> alpha_matrix{};
        std::vector<int> beta_matrix{};
        std::vector<float> row_strategy{};
        std::vector<float> col_strategy{};

        bool is_subgame_singular{true};
        bool must_break{false};
        uint8_t new_row_idx{0};
        uint8_t new_col_idx{0};
        uint8_t new_i{0};
        uint8_t new_j{0};

        void reset_flags_for_alpha_beta_loop() {
            must_break = true;
            new_row_idx = 0;
            new_col_idx = 0;
            new_i = 0;
            new_j = 0;
        }
    };

    struct MatrixPrintData {

        int rows{};
        int cols{};
        std::array<std::array<std::array<float, 4>, 9>, 9> data{};

        void init(const TempData &temp_data, const MatrixNode *matrix_node) {
            size_t e{};
            rows = temp_data.rows;
            cols = temp_data.cols;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    const auto &stats = temp_data.chance_stat_matrix[e];
                    data[i][j][0] = stats.alpha_explored;
                    data[i][j][1] = stats.beta_explored;
                    data[i][j][2] = stats.unexplored;
                    data[i][j][3] = matrix_node->chance_node_matrix.data[e].tries;
                    ++e;
                }
            }
        }
    };

    static void debug_print_chance_matrix(const TempData &temp_data) {
        int entry_idx = 0;
        for (uint8_t row_idx = 0; row_idx < temp_data.rows; ++row_idx) {
            debug_print();
            for (uint8_t col_idx = 0; col_idx < temp_data.cols; ++col_idx) {
                const auto &data = temp_data.chance_stat_matrix[entry_idx];
                debug_print_no_indent(data.alpha_explored, " - ", data.beta_explored, "|", data.unexplored, ", ");
                ++entry_idx;
            }
            debug_print_no_indent('\n');
        }
    }

    struct Branch {

        std::unique_ptr<MatrixNode> matrix_node;
        uint64_t seed;
        float p;

        Branch(const float prob, const uint64_t seed, const bool is_terminal)
            : matrix_node{}, p{static_cast<float>(prob)}, seed{seed} {
        }
    };

    struct ChanceNode {
        std::unordered_map<uint64_t, Branch> branches{};
        std::vector<Branch *> sorted_branches{};
        uint16_t branch_index{};
        uint16_t max_branch_index{};
        uint32_t tries{};

        void total_reset() {
            branches.clear();
            sorted_branches.clear();
            branch_index = 0;
            tries = 0;
        }
    };

    struct ChanceNodeMatrix {
        uint8_t rows, cols;
        ChanceNode *data;
        bool init_ = false;

        void init(const uint8_t rows, const uint8_t cols) {
            this->rows = rows;
            this->cols = cols;
            data = new ChanceNode[rows * cols];
            init_ = true;
        }

        ChanceNode &operator()(uint8_t row_idx, uint8_t col_idx) {
            return data[row_idx * cols + col_idx];
        }

        ~ChanceNodeMatrix() {
            if (rows | cols) {
                delete[] data;
            }
        }
    };

    struct MatrixNode {

        struct ActionProb {
            uint8_t idx;
            uint8_t discrete_prob{};

            ActionProb () {}

            ActionProb(const uint8_t index) : idx{index} {}

            bool operator<(const ActionProb &a) const {
                return discrete_prob < a.discrete_prob;
            }
        };

        struct Solution {
            std::array<ActionProb, 9> action_indices;
            uint8_t boundary{0};
            uint8_t __packing;

            Solution() {}

            void init(const uint8_t size) {
                for (uint8_t i = 0; i < size; ++i) {
                    action_indices[i] = ActionProb{i};
                }
            }

            void add_index(const uint8_t i) {
                assert(boundary >= 0 && boundary < action_indices.size());
                std::swap(action_indices[i], action_indices[boundary++]);
            }

            void remove_index(const uint8_t i) {
                assert(boundary > 0 && boundary <= action_indices.size());
                std::swap(action_indices[i], action_indices[--boundary]);
            }

            uint8_t size() const { return boundary; }
        };

        ChanceNodeMatrix chance_node_matrix;

        Solution I;
        Solution J;

        void prune() {
            for (uint8_t i = I.boundary; i < chance_node_matrix.rows; ++i) {
                for (uint8_t j = J.boundary; j < chance_node_matrix.cols; ++j) {
                    auto &chance_node = chance_node_matrix(I.action_indices[i].idx, J.action_indices[j].idx);
                    chance_node.total_reset();
                }
            }
        }

        void sort_branches_and_reset() {
            for (uint8_t i = 0; i < chance_node_matrix.rows * chance_node_matrix.cols; ++i) {
                // because the branches are added by sampling, this array should be mostly sorted on its own
                // idk if std::sort takes full advantage of this, and that only a tail of the array is unsorted;
                // TODO
                auto &chance_node = chance_node_matrix.data[i];
                chance_node.branch_index = 0;
                chance_node.max_branch_index = chance_node.sorted_branches.size();
                std::sort(
                    chance_node.sorted_branches.begin(),
                    chance_node.sorted_branches.end(),
                    [](const Branch *x, const Branch *y) { return x->p > y->p; });
            }
        }
    };

    struct Search {

        const uint32_t min_tries;
        const uint32_t max_tries;

        float max_unexplored = 0;
        float min_reach_prob_initial = 1;
        float min_reach_prob_base = .1;

        Search(
            const uint32_t min_tries,
            const uint32_t max_tries) : min_tries{min_tries}, max_tries{max_tries} {}

        Search(
            const uint32_t min_tries,
            const uint32_t max_tries,
            const float max_unexplored,
            const float min_reach_prob_initial,
            const float min_reach_prob_base) : min_tries{min_tries}, max_tries{max_tries}, max_unexplored{max_unexplored}, min_reach_prob_initial{min_reach_prob_initial}, min_reach_prob_base{min_reach_prob_base} {}

        Search(

        ) {}

        // calls alpha_beta (basically a wrapper for it) but first checks if not terminal/at max depth
        // helps with boiler plate and reduces matrix node allocations
        void next_solve(
            TempData::ChanceStats &stats,
            Branch* branch, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            head_data.step_forward();
            const float prob = next_temp_data.state.get_prob();
            if (next_temp_data.state.is_terminal()) {
                debug_print("next_solve: terminal: ", next_temp_data.state.get_payoff().get_row_value(), '\n');
                next_temp_data.alpha = next_temp_data.beta = next_temp_data.state.get_payoff().get_row_value();
                ++base_data.terminal_count;
            } else if (
                (next_temp_data.chance_reach_prob = temp_data.chance_reach_prob * prob) < base_data.min_reach_prob ||
                head_data.depth == base_data.max_depth) {
                // if (next_temp_data.chance_reach_prob < base_data.min_reach_prob) {
                //     std::cout << next_temp_data.chance_reach_prob << " " << base_data.min_reach_prob << std::endl;
                // }
                ModelOutput output{};
                next_temp_data.state.get_actions();
                const auto start = std::chrono::high_resolution_clock::now();
                base_data.model->inference(std::move(next_temp_data.state), output);
                const auto end = std::chrono::high_resolution_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                base_data.total_inference_time += duration.count();
                next_temp_data.alpha = next_temp_data.beta = output.value.get_row_value();
                ++base_data.inference_count;
            } else {
                if (!branch->matrix_node) {
                    branch->matrix_node = std::make_unique<MatrixNode>();
                }
                next_temp_data.alpha = 0;
                next_temp_data.beta = 1;
                next_temp_data.chance_stat_matrix.clear();
                next_temp_data.must_break = false;
                debug_print("next_solve: alpha_beta\n");
                if constexpr (debug) {
                    indent += 4;
                }
                this->alpha_beta(branch->matrix_node.get(), base_data, head_data, next_temp_data);
                if constexpr (debug) {
                    indent -= 4;
                }
                ++base_data.alpha_beta_count;
            }
            head_data.step_back();

            stats.unexplored -= prob;
            stats.alpha_explored += next_temp_data.alpha * prob;
            stats.beta_explored += next_temp_data.beta * prob;
            ++base_data.matrix_node_count;
        }

        // query a chance node given a joint action pair. first it returns the branches stored in the tree.
        // then it tries with brute force until max_tries, max_unplored, etc
        Branch *
        try_get_new_branch(const BaseData &base_data, const HeadData &head_data, const TempData &temp_data, TempData &next_temp_data,
                           ChanceNode *chance_node, const uint8_t row_idx, const uint8_t col_idx) const {

            const auto go = [&](const uint64_t seed) {
                next_temp_data.state = temp_data.state;
                next_temp_data.state.randomize_transition(seed);
                next_temp_data.state.apply_actions(
                    next_temp_data.state.row_actions[row_idx],
                    next_temp_data.state.col_actions[col_idx]);
            };

            if (chance_node->branch_index < chance_node->max_branch_index) {
                go(chance_node->sorted_branches[chance_node->branch_index]->seed);
                return chance_node->sorted_branches[chance_node->branch_index++];
            }

            auto &chance_data = temp_data.chance_stat_matrix[row_idx * temp_data.cols + col_idx];
            while (
                !(chance_node->tries >= base_data.max_tries[head_data.depth] ||
                  (chance_node->tries > head_data.min_tries && chance_data.unexplored < base_data.max_unexplored) ||
                  chance_data.unexplored <= 0)) {

                ++chance_node->tries;
                const uint64_t seed = base_data.device->uniform_64();
                go(seed);

                Obs obs = next_temp_data.state.get_obs();
                const uint64_t obs_hash = hash(obs);
                if (chance_node->branches.find(obs_hash) == chance_node->branches.end()) {
                    chance_node->branches.try_emplace(
                        obs_hash,
                        next_temp_data.state.get_prob(), seed, next_temp_data.state.is_terminal());
                    chance_node->sorted_branches.push_back(&chance_node->branches.at(obs_hash));
                    return &chance_node->branches.at(obs_hash);
                }
            }

            return nullptr;
        }

        // query and solve all branches from a chance node
        void solve_chance_node(
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data,
            const uint8_t row_idx, const uint8_t col_idx) const {

            auto &chance_data = temp_data.chance_stat_matrix[row_idx * temp_data.cols + col_idx];
            Branch *new_branch;
            ChanceNode *chance_node = &matrix_node->chance_node_matrix(row_idx, col_idx);
            while (new_branch = try_get_new_branch(base_data, head_data, temp_data, next_temp_data, chance_node, row_idx, col_idx)) {
                next_solve(chance_data, new_branch, base_data, head_data, temp_data, next_temp_data);
                temp_data.is_subgame_singular &= (next_temp_data.alpha == next_temp_data.beta);
            }
            temp_data.is_subgame_singular &= (chance_data.unexplored == 0);
        }

        void discretize_strategies(MatrixNode *matrix_node, const TempData &temp_data) const {
            for (uint8_t i = 0; i < temp_data.row_strategy.size(); ++i) {
                const uint8_t x = 255 * temp_data.row_strategy[i];
                matrix_node->I.action_indices[i].discrete_prob = x;
            }
            for (uint8_t j = 0; j < temp_data.col_strategy.size(); ++j) {
                const uint8_t x = 255 * temp_data.col_strategy[j];
                matrix_node->J.action_indices[j].discrete_prob = x;
            }
        }

        std::pair<float, float>
        compute_subgame_equilibrium(
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            static const int solve_denominator = 80;

            int entry_idx = 0;
            const uint8_t r = matrix_node->I.boundary;
            const uint8_t c = matrix_node->J.boundary;

            if (r <= c) {
                base_data.total_solves[c - 1][r - 1] += 1;
            } else {
                base_data.total_solves[r - 1][c - 1] += 1;
            }
            base_data.total_solves_raw += 1;
            debug_print("solved exactly ", temp_data.is_subgame_singular, '\n');
            debug_print("subgame matrix (unordered):", '\n');

            const auto start = std::chrono::high_resolution_clock::now();

            FloatOneSumOutput output;
            temp_data.row_strategy.resize(r);
            temp_data.col_strategy.resize(c);
            FastInput input;
            input.rows = r;
            input.cols = c;
            input.den = solve_denominator;

            float a;
            float b;

            if (temp_data.is_subgame_singular) {
                temp_data.alpha_matrix.resize(r * c);
                for (uint8_t i = 0; i < r; ++i) {
                    debug_print();
                    for (uint8_t j = 0; j < c; ++j) {
                        const auto &data = temp_data.chance_stat_matrix[matrix_node->I.action_indices[i].idx * temp_data.cols + matrix_node->J.action_indices[j].idx];
                        temp_data.alpha_matrix[entry_idx] = static_cast<int>(data.alpha_explored * solve_denominator);
                        debug_print_no_indent(data.alpha_explored, "|", data.unexplored, ", ");
                        ++entry_idx;
                    }
                    debug_print_no_indent('\n');
                }
                input.data = temp_data.alpha_matrix.data();
                output.row_strategy = temp_data.row_strategy.data();
                output.col_strategy = temp_data.col_strategy.data();
                solve_fast(&input, &output);
                a = b = output.value;
            } else {
                temp_data.alpha_matrix.resize(r * c);
                temp_data.beta_matrix.resize(r * c);
                for (uint8_t i = 0; i < r; ++i) {
                    debug_print();
                    for (uint8_t j = 0; j < c; ++j) {
                        const auto &data = temp_data.chance_stat_matrix[matrix_node->I.action_indices[i].idx * temp_data.cols + matrix_node->J.action_indices[j].idx];
                        temp_data.alpha_matrix[entry_idx] = static_cast<int>(data.alpha_explored * solve_denominator);
                        temp_data.beta_matrix[entry_idx] = static_cast<int>((data.beta_explored + data.unexplored) * solve_denominator);
                        debug_print_no_indent(temp_data.alpha_matrix[entry_idx], " - ", temp_data.beta_matrix[entry_idx], "|", data.unexplored, ", ");
                        ++entry_idx;
                    }
                    debug_print_no_indent('\n');
                }
                std::vector<float> temp{};
                temp.resize(r + c);
                input.data = temp_data.alpha_matrix.data();
                output.row_strategy = temp_data.row_strategy.data();
                output.col_strategy = temp.data();
                solve_fast(&input, &output);
                a = output.value;
                input.data = temp_data.beta_matrix.data();
                output.row_strategy = temp.data();
                output.col_strategy = temp_data.col_strategy.data();
                solve_fast(&input, &output);
                b = output.value;
            }

            const auto end = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            debug_print("Strategies: ", '\n');
            debug_print();
            mpq_vector_print(temp_data.row_strategy);
            debug_print();
            mpq_vector_print(temp_data.col_strategy);
            debug_print("debug matrix (ordered and complete):\n");

            debug_print_chance_matrix(temp_data);

            return {a, b};
        }

        void initialize_submatrix(
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            // init temp data from matrix node solution
            temp_data.chance_stat_matrix.resize(temp_data.rows * temp_data.cols);

            if (matrix_node->I.boundary == 0) {
                matrix_node->chance_node_matrix.init(temp_data.rows, temp_data.cols);
                matrix_node->I.init(temp_data.rows);
                matrix_node->J.init(temp_data.cols);
                matrix_node->I.add_index(base_data.device->random_int(temp_data.rows));
                matrix_node->J.add_index(base_data.device->random_int(temp_data.cols));
                matrix_node->I.action_indices[0].discrete_prob = 255;
                matrix_node->J.action_indices[0].discrete_prob = 255;
            } else {
                for (uint8_t i = 0; i < matrix_node->I.boundary;) {
                    if (matrix_node->I.action_indices[i].discrete_prob <= 20 && (i > 0)) {
                        matrix_node->I.remove_index(i);
                    } else {
                        ++i;
                    }
                }
                for (uint8_t j = 0; j < matrix_node->J.boundary;) {
                    if (matrix_node->J.action_indices[j].discrete_prob <= 20 && (j > 0)) {
                        matrix_node->J.remove_index(j);
                    } else {
                        ++j;
                    }
                }

                assert(matrix_node->I.size() > 0 && matrix_node->J.size() > 0);
            }

            for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                const uint8_t row_idx = matrix_node->I.action_indices[i].idx;
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    const uint8_t col_idx = matrix_node->J.action_indices[j].idx;
                    solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data, row_idx, col_idx);
                }
            }
        }

        void find_row_best_response(
            float &best_response,
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            struct BestResponse {
                const uint8_t i;
                const uint8_t row_idx;
                uint8_t col_idx;
                float value{};
                float total_unexplored{};
                std::vector<float> priority_vector{};
            };

            debug_print("Row BR:", '\n');

            const auto find_nonzero_priority_index = [&](uint8_t &max_index, const BestResponse &data) {
                float max_prio{-1};
                bool nonzero_priority_found{false};
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    if (data.priority_vector[j] > std::max(float{0}, max_prio)) {
                        max_index = j;
                        max_prio = data.priority_vector[j];
                        nonzero_priority_found = true;
                    }
                }
                return nonzero_priority_found;
            };

            const auto can_beat_best_response = [&](BestResponse &data) {
                return data.value + data.total_unexplored >= best_response;
            };

            const auto compute_value = [&](BestResponse &data, uint8_t curr_i) {
                debug_print("row BR; row_idx: ", (int)data.row_idx, '\n');
                // init priority vector
                data.priority_vector.resize(matrix_node->J.boundary);
                {
                    for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                        if (temp_data.col_strategy[j] == 0) {
                            continue;
                        }
                        data.col_idx = matrix_node->J.action_indices[j].idx;
                        const auto &chance_stats = temp_data.chance_stat_matrix[data.row_idx * temp_data.cols + data.col_idx];

                        data.priority_vector[j] =
                            chance_stats.unexplored *
                            temp_data.col_strategy[j];
                        data.total_unexplored += data.priority_vector[j];
                        data.value += chance_stats.beta_explored * temp_data.col_strategy[j];
                    }
                }

                debug_print("priority vector\n");
                debug_print();
                mpq_vector_print(data.priority_vector);

                // explore as much as possible
                Branch *branch;
                for (uint8_t j; can_beat_best_response(data) && find_nonzero_priority_index(j, data);) {
                    data.col_idx = matrix_node->J.action_indices[j].idx;
                    ChanceNode *chance_node = &matrix_node->chance_node_matrix(data.row_idx, data.col_idx);
                    if (branch = try_get_new_branch(base_data, head_data, temp_data, next_temp_data, chance_node, data.row_idx, data.col_idx)) {
                        const float prob = next_temp_data.state.get_prob();
                        auto &chance_data = temp_data.chance_stat_matrix[data.row_idx * temp_data.cols + data.col_idx];
                        next_solve(chance_data, branch, base_data, head_data, temp_data, next_temp_data);
                        const float p = prob * temp_data.col_strategy[j];
                        data.priority_vector[j] -= p;
                        data.value += next_temp_data.beta * prob * temp_data.col_strategy[j];
                        data.total_unexplored -= p;
                    } else {
                        data.priority_vector[j] = 0;
                    }
                }

                data.value += data.total_unexplored;

                debug_print("row response value: ", data.value, '\n');

                if (data.value > best_response || (fuzzy_equals(data.value, best_response) && curr_i >= matrix_node->I.boundary)) {
                    temp_data.new_row_idx = data.row_idx;
                    temp_data.new_i = curr_i;
                    best_response = std::move(data.value);
                    debug_print("new best response - new_i: ", (int)temp_data.new_i, " new_row_idx: ", (int)temp_data.new_row_idx, '\n');
                }
            };

            // best response, finally
            for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                BestResponse data{i, matrix_node->I.action_indices[i].idx};
                compute_value(data, i);
            }

            for (uint8_t i = matrix_node->I.boundary; i < temp_data.rows; ++i) {
                BestResponse data{i, matrix_node->I.action_indices[i].idx};
                compute_value(data, i);
            }

            if (temp_data.new_i >= matrix_node->I.boundary) {
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    this->solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data,
                                            temp_data.new_row_idx, matrix_node->J.action_indices[j].idx);
                }
                temp_data.must_break = false;
            }

            debug_print("================", '\n');
            debug_print('\n');
        }

        void find_col_best_response(
            float &best_response,
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            struct BestResponse {
                const uint8_t j;
                const uint8_t col_idx;
                uint8_t row_idx;
                float value{};
                float total_unexplored{}; // TODO col version doesn't use this
                std::vector<float> priority_vector{};
            };

            const auto find_nonzero_priority_index = [&](uint8_t &max_index, const BestResponse &data) {
                float max_prio{-1};
                bool nonzero_priority_found{false};
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    if (data.priority_vector[i] > std::max(float{0}, max_prio)) {
                        max_index = i;
                        max_prio = data.priority_vector[i];
                        nonzero_priority_found = true;
                    }
                }
                return nonzero_priority_found;
            };

            const auto can_beat_best_response = [&](BestResponse &data) {
                return data.value <= best_response;
            };

            const auto compute_value = [&](BestResponse &data, uint8_t curr_j) {
                data.priority_vector.resize(matrix_node->I.boundary);
                {
                    for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                        if (temp_data.row_strategy[i] == 0) {
                            continue;
                        }
                        data.row_idx = matrix_node->I.action_indices[i].idx;
                        const auto &chance_stats = temp_data.chance_stat_matrix[data.row_idx * temp_data.cols + data.col_idx];
                        data.priority_vector[i] =
                            chance_stats.unexplored *
                            temp_data.row_strategy[i];
                        data.total_unexplored += data.priority_vector[i];
                        data.value += chance_stats.alpha_explored * temp_data.row_strategy[i];
                        // data.value;
                    }
                }

                Branch *branch;
                for (uint8_t i; can_beat_best_response(data) && find_nonzero_priority_index(i, data);) {
                    data.row_idx = matrix_node->I.action_indices[i].idx;
                    ChanceNode *chance_node = &matrix_node->chance_node_matrix(data.row_idx, data.col_idx);
                    if (branch = try_get_new_branch(base_data, head_data, temp_data, next_temp_data, chance_node, data.row_idx, data.col_idx)) {
                        const float prob = next_temp_data.state.get_prob();
                        auto &chance_data = temp_data.chance_stat_matrix[data.row_idx * temp_data.cols + data.col_idx];
                        next_solve(chance_data, branch, base_data, head_data, temp_data, next_temp_data);
                        const float p = prob * temp_data.row_strategy[i];
                        data.priority_vector[i] -= p;
                        data.value += next_temp_data.alpha * p;
                        data.total_unexplored -= p;
                    } else {
                        data.priority_vector[i] = 0;
                    }
                }

                if (data.value < best_response || (fuzzy_equals(data.value, best_response) && curr_j >= matrix_node->J.boundary)) {
                    temp_data.new_col_idx = data.col_idx;
                    temp_data.new_j = curr_j;
                    best_response = std::move(data.value);
                }
            };

            for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                BestResponse data{j, matrix_node->J.action_indices[j].idx};
                compute_value(data, j);
            }

            for (uint8_t j = matrix_node->J.boundary; j < temp_data.cols; ++j) {
                BestResponse data{j, matrix_node->J.action_indices[j].idx};
                compute_value(data, j);
            }

            if (temp_data.new_j >= matrix_node->J.boundary) {
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    this->solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data,
                                            matrix_node->I.action_indices[i].idx, temp_data.new_col_idx);
                }
                temp_data.must_break = false;
            };
        }

        void alpha_beta(MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data) const {
            temp_data.state.get_actions();
            temp_data.rows = temp_data.state.row_actions.size();
            temp_data.cols = temp_data.state.col_actions.size();

            // only allocate for all next function calls, modify in other functions when necessary
            TempData next_temp_data;

            debug_print("ALPHA BETA - depth: ", head_data.depth, '\n');
            debug_print("initializing submatrix");
            this->initialize_submatrix(matrix_node, base_data, head_data, temp_data, next_temp_data);

            while (!fuzzy_equals(temp_data.alpha, temp_data.beta) && !temp_data.must_break) {

                temp_data.reset_flags_for_alpha_beta_loop();

                debug_print("________________", '\n');
                debug_print("alpha: ", temp_data.alpha, '\n');
                debug_print("beta: ", temp_data.beta, '\n');

                debug_print("I: ");
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    debug_print_no_indent((int)matrix_node->I.action_indices[i].idx, ", ");
                }
                debug_print('\n');

                debug_print("J: ");
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    debug_print_no_indent((int)matrix_node->J.action_indices[j].idx, ", ");
                }
                debug_print('\n');

                auto [next_alpha, next_beta] = this->compute_subgame_equilibrium(matrix_node, base_data, head_data, temp_data, next_temp_data);

                if constexpr (debug) {
                    indent += 2;
                }
                this->find_row_best_response(next_beta, matrix_node, base_data, head_data, temp_data, next_temp_data);
                this->find_col_best_response(next_alpha, matrix_node, base_data, head_data, temp_data, next_temp_data);
                if constexpr (debug) {
                    indent -= 2;
                }
                // each of the two prior function calls solves most new chance nodes, but they both miss this corner case when two new actions are added
                // TODO this looks too much like the old code lol
                if (temp_data.new_i >= matrix_node->I.boundary && temp_data.new_j >= matrix_node->J.boundary) {
                    debug_print("float new actions: ", (int)temp_data.new_row_idx, ' ', (int)temp_data.new_col_idx, '\n');
                    this->solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data, temp_data.new_row_idx, temp_data.new_col_idx);
                }
                if (temp_data.new_i >= matrix_node->I.boundary) {
                    matrix_node->I.add_index(temp_data.new_i);
                }
                if (temp_data.new_j >= matrix_node->J.boundary) {
                    matrix_node->J.add_index(temp_data.new_j);
                }
                if (next_alpha > temp_data.alpha || next_beta < temp_data.beta) {
                    temp_data.must_break = false;
                }

                temp_data.alpha = std::max(next_alpha, temp_data.alpha);
                temp_data.beta = std::min(next_beta, temp_data.beta);

                debug_print("next_a: ", next_alpha, " next_b: ", next_beta, '\n');
                debug_print("new row idx: ", (int)temp_data.new_row_idx, " new col idx: ", (int)temp_data.new_col_idx, " must_break:", temp_data.must_break, '\n');
                debug_print("________________", '\n');
                debug_print('\n');
            }

            discretize_strategies(matrix_node, temp_data);
            matrix_node->sort_branches_and_reset();
            debug_print("END ALPHA BETA\n");
            debug_print_chance_matrix(temp_data);

            temp_data.alpha = std::min(std::max(temp_data.alpha, 0.0f), 1.0f);
            temp_data.beta = std::min(std::max(temp_data.beta, 0.0f), 1.0f);
            if (temp_data.alpha > temp_data.beta) {
                std::swap(temp_data.alpha, temp_data.beta);
            }
            // temp_data.alpha = temp_data.beta = (temp_data.alpha + temp_data.beta )/2;
        }

        struct Output {
            float alpha;
            float beta;
            std::vector<float> row_strategy{};
            std::vector<float> col_strategy{};
            std::vector<size_t> times{};
            std::vector<int64_t> times_without_inference{};
            size_t total_solves[9][9];
            size_t total_solves_raw{};

            std::vector<size_t> matrix_node_count{};
            std::vector<size_t> alpha_beta_count{};
            std::vector<size_t> terminal_count{};
            std::vector<size_t> inference_count{};

            MatrixPrintData matrix_print_data{};
        };

        template <typename T>
        Output run(const std::vector<T> &depths, PRNG &device, const State &state, Model &model, MatrixNode &node) const {

            Output output;
            for (const auto depth : depths) {

                std::vector<uint32_t> max_tries_vec{};
                max_tries_vec.resize(depth);
                const float factor = 1.414;
                uint32_t t = max_tries;
                float min_reach_prob = min_reach_prob_initial;
                for (int i = depth - 1; i >= 0; --i) {
                    max_tries_vec[i] = (t);
                    const float w = t * factor;
                    t = static_cast<uint32_t>(w);
                    min_reach_prob *= min_reach_prob_base;
                }

                BaseData base_data{depth, &device, &model, min_tries, max_tries_vec, max_unexplored, min_reach_prob};
                HeadData head_data{min_tries, max_tries};
                TempData temp_data{state};

                const auto start = std::chrono::high_resolution_clock::now();
                this->alpha_beta(&node, base_data, head_data, temp_data);
                const auto end = std::chrono::high_resolution_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

                output.alpha = temp_data.alpha;
                output.beta = temp_data.beta;
                output.row_strategy.resize(temp_data.rows);
                for (uint8_t i = 0; i < temp_data.row_strategy.size(); ++i) {
                    output.row_strategy[node.I.action_indices[i].idx] = temp_data.row_strategy[i];
                }
                output.col_strategy.resize(temp_data.cols);
                for (uint8_t j = 0; j < temp_data.col_strategy.size(); ++j) {
                    output.col_strategy[node.J.action_indices[j].idx] = temp_data.col_strategy[j];
                }

                output.matrix_node_count.push_back(base_data.matrix_node_count);
                output.alpha_beta_count.push_back(base_data.alpha_beta_count);
                output.terminal_count.push_back(base_data.terminal_count);
                output.inference_count.push_back(base_data.inference_count);

                output.times.push_back(duration.count());

                const int64_t time_without_inference =
                    static_cast<int64_t>(duration.count()) - static_cast<int64_t>(base_data.total_inference_time);
                output.times_without_inference.push_back(time_without_inference);

                std::memcpy(output.total_solves, base_data.total_solves, sizeof(size_t) * 81);
                output.total_solves_raw = base_data.total_solves_raw;

                output.matrix_print_data.init(temp_data, &node);
            }

            return output;
        }

        Output run(const uint32_t depth, PRNG &device, const State &state, Model &model, MatrixNode &node) const {
            std::vector<uint32_t> depths{};
            for (uint32_t d = 1; d <= depth; ++d) {
                depths.push_back(d);
            }
            return this->run(depths, device, state, model, node);
        }
    };
};
