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

  - In the tree
    we try to not put things here. only stuff needed between depth/chance iteration
    namely: I, J (which define subgames and also store old strategies therein)

  - In the Head
    either constant for the search or can be inc/dec as the search traverses the tree
    : device, base state, model, base tries/max_unexplored etc

  - In the body
    data thats local to each node being solved.
*/

struct State {
  void randomize_transition(uint64_t seed) {}
  std::vector<uint8_t> row_actions{}, col_actions{};
  template <typename T>
  void apply_actions(T, T) {}
  bool is_terminal() { return {}; }
  void get_actions() {}
};

struct Device {
  uint64_t uniform_64() {
    return rand();
  }
};

struct MatrixNode;

struct Branch {
  std::unique_ptr<MatrixNode> matrix_node;
  double prob;
  uint64_t seed;

  Branch(const double prob, const uint64_t seed) : matrix_node{std::make_unique<MatrixNode>()}, prob{prob}, seed{seed} {}
};

struct HeadData {
  Device *device;
  uint32_t min_tries;
  uint32_t max_tries;
  double max_unexplored;
  uint32_t depth;
  uint32_t max_depth;
};

struct BodyData {
  State state{};
  uint8_t row_idx;
  uint8_t col_idx;
};

struct ChanceNode {
  std::unordered_map<uint64_t, Branch> branches{};
  std::vector<Branch *> branches_ordered_by_prob{};
  int preexisting_branch_index{};
  uint32_t tries{};

  mpq_class alpha_explored{};
  mpq_class beta_explored{};
  mpq_class unexplored{};

  Branch *try_get_new_branch(const HeadData &head_data, const BodyData &body_data) {
    if (preexisting_branch_index < branches_ordered_by_prob.size()) {
      return branches_ordered_by_prob[preexisting_branch_index++];
    }
    while (
        tries <= head_data.max_tries &&
        (tries <= head_data.min_tries || unexplored >= head_data.max_unexplored) &&
        unexplored > 0) {
      State copy{body_data.state};
      const uint64_t seed = head_data.device->uniform_64();
      copy.randomize_transition(seed);
      copy.apply_actions(
          copy.row_actions[body_data.row_idx],
          copy.col_actions[body_data.col_idx]);
      uint64_t obs{}; // TODO
      double prob{};
      if (branches.find(obs) == branches.end()) {
        branches.try_emplace(obs, prob, seed);
        Branch *new_branch = &branches.at(seed);
        return new_branch;
      }
    }

    return nullptr;
  }

  void reset_stats_for_reexploration() {
    alpha_explored = beta_explored = 0;
    unexplored = 1;
    std::sort(branches_ordered_by_prob.begin(), branches_ordered_by_prob.end(), [](const Branch *x, const Branch *y) { return x->prob > y->prob; });
    preexisting_branch_index = 0;
  }

  void total_reset() {
    branches.clear();
    branches_ordered_by_prob.clear();
    preexisting_branch_index = 0;
    tries = 0;
    alpha_explored = 0;
    beta_explored = 0;
    unexplored = 1;
  }

  void insert_branch(const Branch *const branch) {
    // branches_ordered_by_prob.insert();
  }
};

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

  template <typename ActionData = uint8_t>
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

    void clean_up_chance_nodes() {
    }

    // uint8_t operator[](uint8_t i) const {
    //     return
    // }
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

  void solve_chance_node(ChanceNode &chance_node, const HeadData& head_data, const BodyData &body_data) {
    Branch *new_branch;
    while (new_branch = chance_node.try_get_new_branch(head_data, body_data)) {
      auto [a, b] = this->alpha_beta_wrapper(body_data, head_data);
      chance_node.unexplored -= new_branch->prob;
      chance_node.unexplored.canonicalize();

    }
  }

  void initialize_submatrix(
      MatrixNode *matrix_node) {
    for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {
      for (uint8_t j = 0; j < matrix_node->J.boundary; ++j) {
      }
      this->solve_chance_node(matrix_node->chance_node_matrix(i, j));
    }
  }

  // pass a reference to best score
  void row_best_response(mpq_class &alpha, MatrixNode *matrix_node) {
    struct BestResponse {
      struct 
      std::vector<mpq_class> unexplored{};
      std::vector<mpq_class> unexplored{};
    
    };

    for (uint8_t i = 0; i < matrix_node->I.boundary; ++i) {

    }

    for (uint8_t i = matrix_node->I.boundary; i < matrix_node->chance_node_matrix.rows; ++i) {

    }
  }

  void col_best_response(mpq_class &beta, MatrixNode *matrix_node) {
    struct BestResponse {
    };
  }

  std::pair<mpq_class, mpq_class> solve_subgame() { return {{}, {}}; }

  std::pair<mpq_class, mpq_class>
  alpha_beta_wrapper(
      MatrixNode *parent_node,
      BodyData &body_data,
      HeadData &head_data) {
    if (body_data.state.is_terminal()) {
      return {{}, {}};
    }

    if (head_data.depth >= head_data.max_depth) {
      return {{}, {}};
    }
    // TODO
    auto matrix_node = parent_node;
    alpha_beta(matrix_node, body_data, head_data);
  }
  std::pair<mpq_class, mpq_class>
  alpha_beta(
      MatrixNode *matrix_node,
      BodyData &body_data,
      HeadData &head_data) {

    mpq_class alpha{0};
    mpq_class beta{1};
    body_data.state.get_actions();

    BodyData data{};
    // copy over state or w/e

    this->initialize_submatrix();
    // if new node, expand. otherwise populate the old NE sub matrix

    while (alpha < beta) {

      auto [a0, b0] = this->solve_subgame();

      this->row_best_response(a0);
      this->col_best_response(b0);

      alpha = std::min(b0, alpha);
      beta = std::min(a0, beta);
    }

    for (uint8_t i = 0; i < matrix_node->chance_node_matrix.rows * matrix_node->chance_node_matrix.cols; ++i) {
      matrix_node->chance_node_matrix.data[i].reset_stats_for_reexploration();
    }

    return {alpha, beta};
  }
};

int main() {
  srand(time(NULL));

  MatrixNode node{};
  return 0;
}