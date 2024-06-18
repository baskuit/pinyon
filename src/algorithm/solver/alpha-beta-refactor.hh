#pragma once

#include <libpinyon/lrslib.hh>

#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

static int indent = 0;

template <typename Types, bool debug = true>
struct AlphaBetaRefactor {

    static void mpq_vector_print(const std::vector<mpq_class> &input) {
        if constexpr (debug) {
            for (int i = 0; i < input.size(); ++i) {
                std::cout << input[i].get_str() << ", ";
            }
            std::cout << std::endl;
        }
    }

    template <typename... T>
    static void debug_print(T ...args) {
        if constexpr (debug) {
            for (int i = 0; i < indent; ++i) {
                std::cout << " ";
            }
            ((std::cout << args), ...);
        }
    }


    template <typename... T>
    static void debug_print_no_indent(T ...args) {
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
        return static_cast<uint64_t>(obs);
    }

    struct MatrixNode;

    struct BaseData {
        uint32_t max_depth;
        PRNG *device;
        Model *model;
        const State state;
        uint32_t min_tries;
        uint32_t max_tries;
        double max_unexplored;
        double min_chance_prob;

        BaseData(
            uint32_t max_depth,
            PRNG *device,
            Model *model,
            const State &state,
            uint32_t min_tries,
            uint32_t max_tries,
            double max_unexplored,
            double min_chance_prob)
            : max_depth{max_depth}, device{device}, model{model}, state{state}, min_tries{min_tries}, max_tries{max_tries}, max_unexplored{max_unexplored}, min_chance_prob{min_chance_prob} {}
    };

    struct HeadData {
        uint32_t min_tries;
        uint32_t max_tries;
        double max_unexplored;
        double min_chance_prob;
        uint32_t depth{0};

        HeadData(
            uint32_t min_tries,
            uint32_t max_tries) : min_tries{min_tries}, max_tries{max_tries}, max_unexplored{0}, min_chance_prob{0} {}

        HeadData(
            uint32_t min_tries,
            uint32_t max_tries,
            double max_unexplored,
            double min_chance_prob) : min_tries{min_tries}, max_tries{max_tries}, max_unexplored{max_unexplored}, min_chance_prob{min_chance_prob} {}

        void step_forward() {
            ++depth;
        }
        void step_back() {
            --depth;
        }
    };

    struct TempData {
        State state{prng{0}, 1, 1, 1, 1};
        uint16_t rows, cols;
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

        bool is_solved_exactly{true};
        bool must_break{false};
        uint8_t new_row_idx{0xFF};
        uint8_t new_col_idx{0xFF};
        uint8_t new_i{0xFF};
        uint8_t new_j{0xFF};

        void reset_flags_for_alpha_beta_loop() {
            must_break = true;
            new_row_idx = 0xFF;
            new_col_idx = 0xFF;
            new_i = 0xFF;
            new_j = 0xFF;
            // row_strategy.clear();
            // col_strategy.clear();
            // alpha_matrix.clear();
            // beta_matrix.clear();
        }
    };

    struct Branch {

        std::unique_ptr<MatrixNode> matrix_node;
        uint64_t seed;
        float p;

        Branch(const double prob, const uint64_t seed, const bool is_terminal)
            : matrix_node{}, p{static_cast<float>(prob)}, seed{seed} {
            if (!is_terminal) {
                matrix_node = std::make_unique<MatrixNode>();
            }
        }
    };

    struct ChanceNode {
        std::unordered_map<uint64_t, Branch> branches{};
        std::vector<Branch *> sorted_branches{};
        int branch_index{};
        uint32_t tries{};

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

            ActionProb(const uint8_t index) : idx{index} {}

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

            void init(const uint8_t size) {
                for (uint8_t i = 0; i < size; ++i) {
                    action_indices.push_back(i);
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
                auto &chance_data = chance_node_matrix.data[i];
                chance_data.branch_index = 0;
                std::sort(
                    chance_data.sorted_branches.begin(),
                    chance_data.sorted_branches.end(),
                    [](const Branch *x, const Branch *y) { return x->p > y->p; });
            }
        }
    };

    struct Search {

        // calls alpha_beta (basically a wrapper for it) but first checks if not terminal/at max depth
        // helps with boiler plate and reduces matrix node allocations
        void next_solve(
            TempData::ChanceStats &stats,
            MatrixNode *next_matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            indent += 4;
            const mpq_class prob = next_temp_data.state.get_prob();
            stats.unexplored -= prob;
            if (next_temp_data.state.is_terminal()) {
                debug_print("next_solve: terminal: ", next_temp_data.state.get_payoff().get_row_value().get_d(), '\n');
                next_temp_data.alpha = next_temp_data.beta = next_temp_data.state.get_payoff().get_row_value();
            } else if (head_data.min_chance_prob > prob) { // TODO init min_chance_prob
                next_temp_data.alpha = 0;
                next_temp_data.beta = 1;
            } else if (head_data.depth == base_data.max_depth) {
                ModelOutput output{};
                base_data.model->inference(std::move(next_temp_data.state), output);
                next_temp_data.alpha = next_temp_data.beta = output.value.get_row_value();
            } else {
                next_temp_data.alpha = 0;
                next_temp_data.beta = 1;
                next_temp_data.chance_stat_matrix.clear();
                debug_print("next_solve: alpha_beta\n");
                this->alpha_beta(next_matrix_node, base_data, head_data, next_temp_data);
            }
            stats.alpha_explored += next_temp_data.alpha * prob;
            stats.beta_explored += next_temp_data.beta * prob;
            temp_data.is_solved_exactly &= (next_temp_data.alpha == next_temp_data.beta);
            indent -= 4;
        }

        // query a chance node given a joint action pair. first it returns the branches stored in the tree.
        // then it tries with brute force until max_tries, max_unplored, etc
        Branch *
        try_get_new_branch(const BaseData &base_data, const HeadData &head_data, const TempData &temp_data, TempData &next_temp_data,
                           ChanceNode *chance_node, const uint8_t row_idx, const uint8_t col_idx) const {

            const auto go = [&](const uint64_t seed) {
                next_temp_data.state = temp_data.state;
                next_temp_data.state.randomize_transition(seed);
                debug_print("APPLY_ACTIONS: ", (int)row_idx, " ", (int)col_idx, '\n');
                next_temp_data.state.apply_actions(
                    next_temp_data.state.row_actions[row_idx],
                    next_temp_data.state.col_actions[col_idx]);
            };

            if (chance_node->branch_index < chance_node->sorted_branches.size()) {
                go(chance_node->sorted_branches[chance_node->branch_index]->seed);
                return chance_node->sorted_branches[chance_node->branch_index++];
            }

            auto &chance_data = temp_data.chance_stat_matrix[row_idx * temp_data.cols + col_idx];
            while (
                !(chance_node->tries >= base_data.max_tries ||
                  (chance_node->tries > head_data.min_tries && chance_data.unexplored < head_data.max_unexplored) ||
                  chance_data.unexplored <= 0)) {

                ++chance_node->tries;
                const uint64_t seed = base_data.device->uniform_64();
                go(seed);

                Obs obs = next_temp_data.state.get_obs();
                const uint64_t obs_hash = hash(obs);
                if (chance_node->branches.find(obs_hash) == chance_node->branches.end()) {
                    chance_node->branches.try_emplace(
                        obs_hash,
                        next_temp_data.state.get_prob().get_d(), seed, next_temp_data.state.is_terminal());
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
                next_solve(chance_data, new_branch->matrix_node.get(), base_data, head_data, temp_data, next_temp_data);
            }
        }

        std::pair<mpq_class, mpq_class>
        compute_subgame_equilibrium(
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            int entry_idx = 0;
            const uint8_t r = matrix_node->I.boundary;
            const uint8_t c = matrix_node->J.boundary;
            // TODO discretizing is broken since small strats will be clipped to 0 and be skipped for best response
            const auto discretize_strategies = [&](){
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    const uint8_t row_idx = matrix_node->I.action_indices[i].idx;
                    const uint8_t x = 255 * temp_data.row_strategy[i].get_d();
                    matrix_node->I.action_indices[i].discrete_prob = x;
                }
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    const uint8_t col_idx = matrix_node->J.action_indices[j].idx;
                    const uint8_t x = 255 * temp_data.col_strategy[j].get_d();
                    matrix_node->J.action_indices[j].discrete_prob = x;
                }
            };

            debug_print("solved exactly ", temp_data.is_solved_exactly, '\n');
            debug_print("subgame matrix (unordered):", '\n');

            if (temp_data.is_solved_exactly) {
                temp_data.alpha_matrix.resize(r * c);
                for (uint8_t i = 0; i < r; ++i) {
                    debug_print();
                    for (uint8_t j = 0; j < c; ++j) {
                        const auto &data = temp_data.chance_stat_matrix[matrix_node->I.action_indices[i].idx * temp_data.cols + matrix_node->J.action_indices[j].idx];
                        temp_data.alpha_matrix[entry_idx] = data.alpha_explored;
                        debug_print_no_indent(temp_data.alpha_matrix[entry_idx].get_str(), "|", data.unexplored.get_d(), ", ");
                        ++entry_idx;
                    }
                    debug_print('\n');
                }

                const auto a = LRSNash::solve(r, c, temp_data.alpha_matrix, temp_data.row_strategy, temp_data.col_strategy);
                discretize_strategies();

                debug_print("Strategies: ", '\n');
                debug_print();
                mpq_vector_print(temp_data.row_strategy);
                debug_print();
                mpq_vector_print(temp_data.col_strategy);


                {
                    debug_print("debug matrix (ordered and complete):\n");

                    int entry_idx = 0;
                    for (uint8_t row_idx = 0; row_idx < temp_data.rows; ++row_idx) {
                        debug_print();
                        for (uint8_t col_idx = 0; col_idx < temp_data.cols; ++col_idx) {
                            const auto &data = temp_data.chance_stat_matrix[entry_idx];
                            debug_print_no_indent(data.alpha_explored.get_str(), "|", data.unexplored.get_d(), ", ");
                            ++entry_idx;
                        }
                        debug_print_no_indent('\n');
                    }
                };

                return {a, a};
            } else {
                temp_data.alpha_matrix.resize(r * c);
                temp_data.beta_matrix.resize(r * c);
                for (uint8_t i = 0; i < r; ++i) {
                    for (uint8_t j = 0; j < c; ++j) {
                        const auto &data = temp_data.chance_stat_matrix[matrix_node->I.action_indices[i].idx * temp_data.cols + matrix_node->J.action_indices[j].idx];
                        temp_data.alpha_matrix[entry_idx] = data.alpha_explored;
                        temp_data.beta_matrix[entry_idx] = data.beta_explored + data.unexplored;
                        ++entry_idx;
                    }
                }

                std::vector<mpq_class> temp;
                const auto a = LRSNash::solve(temp_data.rows, temp_data.cols, temp_data.alpha_matrix, temp_data.row_strategy, temp);
                temp.clear();
                const auto b = LRSNash::solve(temp_data.rows, temp_data.cols, temp_data.beta_matrix, temp, temp_data.col_strategy);
                discretize_strategies();
                return {a, b};
            }
        }

        void initialize_submatrix(
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            // init temp data from matrix node solution
            if (matrix_node->I.boundary == 0) {
                temp_data.chance_stat_matrix.resize(temp_data.rows * temp_data.cols);
                matrix_node->chance_node_matrix.init(temp_data.rows, temp_data.cols);
                matrix_node->I.init(temp_data.rows);
                matrix_node->J.init(temp_data.cols);
                matrix_node->I.add_index(0);
                matrix_node->J.add_index(0);
                matrix_node->I.action_indices[0].discrete_prob = 0xFF;
                matrix_node->J.action_indices[0].discrete_prob = 0xFF;
            } else {
                assert(false);
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    if (matrix_node->I.action_indices[i].discrete_prob == 0) {
                        matrix_node->I.remove_index(i);
                        --i;
                    }
                }
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    if (matrix_node->J.action_indices[j].discrete_prob == 0) {
                        matrix_node->J.remove_index(j);
                        --j;
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
            mpq_class &best_response,
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            struct BestResponse {
                const uint8_t i;
                const uint8_t row_idx;
                uint8_t col_idx;
                uint8_t row_idx_in_I_already;
                mpq_class value{};
                mpq_class total_unexplored{};
                std::vector<mpq_class> priority_vector{};
            };

            // debug_print("================", '\n');

            const auto find_nonzero_priority_index = [&](uint8_t &max_index, const BestResponse &data) {
                mpq_class max_prio{-1};
                bool nonzero_priority_found{false};
                for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                    if (data.priority_vector[j] > std::max(mpq_class{}, max_prio)) {
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
                // debug_print("row BR; row_idx: ", (int)data.row_idx, '\n');
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
                        // data.value.canonicalize();
                    }
                }

                // debug_print("priority vector", '\n');
                // debug_print();
                // mpq_vector_print(data.priority_vector);

                // explore as much as possible
                Branch *branch;
                for (uint8_t j; can_beat_best_response(data) && find_nonzero_priority_index(j, data);) {
                    data.col_idx = matrix_node->J.action_indices[j].idx;
                    ChanceNode *chance_node = &matrix_node->chance_node_matrix(data.row_idx, data.col_idx);
                    if (branch = try_get_new_branch(base_data, head_data, temp_data, next_temp_data, chance_node, data.row_idx, data.col_idx)) {
                        const mpq_class prob = next_temp_data.state.get_prob();
                        auto &chance_data = temp_data.chance_stat_matrix[data.row_idx * temp_data.cols + data.col_idx];
                        next_solve(chance_data, branch->matrix_node.get(), base_data, head_data, temp_data, next_temp_data);
                        const mpq_class p = prob * temp_data.col_strategy[j];
                        data.priority_vector[j] -= p;
                        data.value += next_temp_data.beta * prob * temp_data.col_strategy[j];
                        data.total_unexplored -= p;
                    } else {
                        data.priority_vector[j] = 0;
                    }
                }

                data.value += data.total_unexplored;
                data.value.canonicalize();

                // debug_print("value: ", data.value.get_d(), '\n');

                if (data.value > best_response || (data.value == best_response && curr_i >= matrix_node->I.boundary)) {
                    temp_data.new_row_idx = data.row_idx;
                    temp_data.new_i = curr_i;
                    best_response = std::move(data.value);
                }
            };

            // best response, finally
            {
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    BestResponse data{i, matrix_node->I.action_indices[i].idx};
                    data.row_idx_in_I_already = true;
                    compute_value(data, i);
                }

                for (uint8_t i = matrix_node->I.boundary; i < temp_data.rows; ++i) {
                    BestResponse data{i, matrix_node->I.action_indices[i].idx};
                    data.row_idx_in_I_already = false;
                    compute_value(data, i);
                }

                if (temp_data.new_row_idx != 0xFF) {
                    for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
                        this->solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data,
                                                temp_data.new_row_idx, matrix_node->J.action_indices[j].idx);
                    }
                    temp_data.must_break = false;
                }

                // debug_print("================", '\n');
                // debug_print('\n');
            };
        }

        void find_col_best_response(
            mpq_class &best_response,
            MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data, TempData &next_temp_data) const {

            struct BestResponse {
                const uint8_t j;
                const uint8_t col_idx;
                uint8_t row_idx;
                uint8_t col_idx_in_I_already;
                mpq_class value{};
                mpq_class total_unexplored{}; // TODO col version doesn't use this
                std::vector<mpq_class> priority_vector{};
            };

            const auto find_nonzero_priority_index = [&](uint8_t &max_index, const BestResponse &data) {
                mpq_class max_prio{-1};
                bool nonzero_priority_found{false};
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    if (data.priority_vector[i] > std::max(mpq_class{}, max_prio)) {
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
                        // data.value.canonicalize();
                    }
                }

                Branch *branch;
                for (uint8_t i; can_beat_best_response(data) && find_nonzero_priority_index(i, data);) {
                    data.row_idx = matrix_node->I.action_indices[i].idx;
                    ChanceNode *chance_node = &matrix_node->chance_node_matrix(data.row_idx, data.col_idx);
                    if (branch = try_get_new_branch(base_data, head_data, temp_data, next_temp_data, chance_node, data.row_idx, data.col_idx)) {
                        const mpq_class prob = next_temp_data.state.get_prob();
                        auto &chance_data = temp_data.chance_stat_matrix[data.row_idx * temp_data.cols + data.col_idx];
                        next_solve(chance_data, branch->matrix_node.get(), base_data, head_data, temp_data, next_temp_data);
                        const mpq_class p = prob * temp_data.row_strategy[i];
                        data.priority_vector[i] -= p;
                        data.value += next_temp_data.alpha * p;
                        data.total_unexplored -= p;
                    } else {
                        data.priority_vector[i] = 0;
                    }
                }

                data.value.canonicalize();

                if (data.value < best_response || (data.value == best_response && curr_j >= matrix_node->J.boundary)) {
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

            if (temp_data.new_col_idx != 0xFF) {
                for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
                    this->solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data,
                                            matrix_node->I.action_indices[i].idx, temp_data.new_col_idx);
                }
                temp_data.must_break = false;
            };
        }

        void alpha_beta(MatrixNode *matrix_node, BaseData &base_data, HeadData &head_data, TempData &temp_data) const {

            head_data.step_forward();

            temp_data.state.get_actions();
            temp_data.rows = temp_data.state.row_actions.size();
            temp_data.cols = temp_data.state.col_actions.size();

            // only allocate for all next function calls, modify in other functions when necessary
            TempData next_temp_data;

            this->initialize_submatrix(matrix_node, base_data, head_data, temp_data, next_temp_data);

            while (temp_data.alpha < temp_data.beta && !temp_data.must_break) {

                temp_data.reset_flags_for_alpha_beta_loop();

                auto [next_beta, next_alpha] = this->compute_subgame_equilibrium(matrix_node, base_data, head_data, temp_data, next_temp_data);

                {
                    debug_print("________________", '\n');
                    debug_print("alpha: ", temp_data.alpha.get_d(), '\n');
                    debug_print("beta: ", temp_data.beta.get_d(), '\n');

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

                };

                next_beta = mpq_class{0};
                next_alpha = mpq_class{1};

                this->find_row_best_response(next_beta, matrix_node, base_data, head_data, temp_data, next_temp_data);
                this->find_col_best_response(next_alpha, matrix_node, base_data, head_data, temp_data, next_temp_data);


                // each of the two prior function calls solves most new chance nodes, but they both miss this corner case when two new actions are added
                if (temp_data.new_i >= matrix_node->I.boundary && temp_data.new_i != 0xFF) {
                    matrix_node->I.add_index(temp_data.new_i);
                }
                if (temp_data.new_j >= matrix_node->J.boundary && temp_data.new_j != 0xFF) {
                    matrix_node->J.add_index(temp_data.new_j);
                }
                if (temp_data.new_row_idx != 0xFF && temp_data.new_col_idx != 0xFF) {
                    debug_print("double new actions: ", (int)temp_data.new_row_idx, ' ', (int)temp_data.new_col_idx, '\n');
                    this->solve_chance_node(matrix_node, base_data, head_data, temp_data, next_temp_data, temp_data.new_row_idx, temp_data.new_col_idx);
                }

                temp_data.alpha = std::max(next_alpha, temp_data.alpha);
                temp_data.beta = std::min(next_beta, temp_data.beta);


                {
                    debug_print("next_a: ", next_alpha.get_d(), " next_b: ", next_beta.get_d(), '\n');
                    debug_print("new row idx: ", (int)temp_data.new_row_idx, " new col idx: ", (int)temp_data.new_col_idx, " must_break:", temp_data.must_break, '\n');
                    debug_print("________________", '\n');
                    debug_print('\n');
                };
            }

            matrix_node->sort_branches_and_reset();

            head_data.step_back();
        }

        struct Output {
            mpq_class alpha;
            mpq_class beta;
            std::vector<size_t> counts{};
            std::vector<size_t> times{};
        };

        template <typename T>
        Output run(const std::vector<T> &depths, PRNG &device, const State &state, Model &model, MatrixNode &node) const {
            Output output;

            for (const auto depth : depths) {
                BaseData base_data{depth, &device, &model, state, 1, 1, 0, 0};

                HeadData head_data{1, 1};

                TempData temp_data{state};

                const auto start = std::chrono::high_resolution_clock::now();
                this->alpha_beta(&node, base_data, head_data, temp_data);
                const auto end = std::chrono::high_resolution_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                output.alpha = temp_data.alpha;
                output.beta = temp_data.beta;
                // output.counts.push_back(node.count_matrix_nodes());
                output.counts.push_back(0);
                output.times.push_back(duration.count());
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
