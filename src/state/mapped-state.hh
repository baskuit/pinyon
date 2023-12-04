#include <state/state.hh>

#include <tree/tree-debug.hh>

#include <memory>
#include <unordered_map>
#include <format>
#include <sstream>
#include <filesystem>
/*

Expands a shared tree around the input state by brute-forcing chance nodes.
This tree only stores the witnessed chance actions and their emprical probs.

Much like `FullTraversal`, this is exactly a regular Types::State but with the shared data as well.
When we  call `apply_actions` and transition to the of the shared tree, we mark as terminal.
No search is done at that moment, but calling `get_payoff` will now perform a search with a disposable root node,
and the model and search objects that were copied during initialization

*/

std::string arrayToString(const std::array<uint8_t, 16> &arr)
{
    std::ostringstream oss;
    for (size_t i = 0; i < arr.size(); ++i)
    {
        oss << static_cast<int>(arr[i]);
        if (i < arr.size() - 1)
        {
            oss << ",";
        }
    }
    return oss.str();
}

void createNestedDirectories(const std::string &basePath, const std::vector<std::string> &directoryNames)
{
    std::filesystem::path currentPath = basePath;

    for (const auto &dirName : directoryNames)
    {
        currentPath /= dirName;                         // Append the next directory to the path
        std::filesystem::create_directory(currentPath); // Create the directory
    }
}

template <
    typename Types,
    bool empirical = true>
struct MappedState : Types::TypeList
{

    struct MatrixStats
    {
    };

    struct ChanceData
    {
        size_t count = 0;
        typename Types::Prob prob;
        typename Types::Seed seed;
    };

    struct ChanceStats
    {
        std::unordered_map<
            typename Types::Obs,
            ChanceData,
            typename Types::ObsHash>
            map{};
        size_t count = 0;
    };

    using NodeTypes = DefaultNodes<
        Types,
        MatrixStats,
        ChanceStats,
        typename Types::VectorAction>; // actions are stored in the matrix node

    using MatrixNode = NodeTypes::MatrixNode;
    using ChanceNode = NodeTypes::ChanceNode;

    static void run(
        const size_t depth,
        const size_t tries,
        Types::PRNG &device,
        NodeTypes::State &state,
        MatrixNode *matrix_node)
    {

        if (depth <= 0 || state.is_terminal())
        {
            matrix_node->set_terminal();
            return;
        }

        state.get_actions(matrix_node->row_actions, matrix_node->col_actions);

        for (int row_idx = 0; row_idx < matrix_node->row_actions.size(); ++row_idx)
        {
            for (int col_idx = 0; col_idx < matrix_node->col_actions.size(); ++col_idx)
            {
                ChanceNode *chance_node = matrix_node->access(row_idx, col_idx);

                typename Types::Prob total_prob{};

                for (int t = 0; t < tries && total_prob < typename Types::Prob{1}; ++t)
                {

                    typename Types::State state_copy = state;
                    typename Types::Seed seed{device.uniform_64()};
                    state_copy.randomize_transition(seed);
                    state_copy.apply_actions(
                        matrix_node->row_actions[row_idx],
                        matrix_node->col_actions[col_idx]);

                    MatrixNode *matrix_node_next = chance_node->access(state_copy.get_obs());
                    auto &chance_data = chance_node->stats.map[state_copy.get_obs()];

                    if (chance_data.count == 0)
                    {
                        chance_data.prob = state_copy.prob;
                        chance_data.seed = seed;
                        run(
                            depth - 1,
                            tries,
                            device,
                            state_copy,
                            matrix_node_next);
                        total_prob += state_copy.prob;
                    }
                    ++chance_data.count;
                    ++chance_node->stats.count;
                }
            }
        }
    }

    class State : public Types::State
    {
    public:
        const MatrixNode
            *node;
        std::shared_ptr<const MatrixNode>
            explored_tree;
        const typename Types::Model
            model;
        const typename Types::Search
            search;
        const size_t iterations;

        State(
            const size_t depth,
            const size_t tries,
            const size_t iterations,
            Types::PRNG &device,
            Types::State &state,
            const Types::Model &model,
            const Types::Search &search)
            : Types::State{state},
              iterations{iterations},
              model{model},
              search{search}
        {
            auto temp_tree = std::make_shared<MatrixNode>();
            node = temp_tree.get();
            run(depth, tries, device, state, temp_tree.get());
            // fill(state, temp_tree.get(), {"/home/user/Desktop/pkmn-pinyon-test/tree"});
            explored_tree = temp_tree;
            this->row_actions = node->row_actions;
            this->col_actions = node->col_actions;
        }

        void get_actions()
        {
            this->row_actions = node->row_actions;
            this->col_actions = node->col_actions;
        }

        void get_actions(
            Types::VectorAction row_actions,
            Types::VectorAction col_actions)
        {
            row_actions = node->row_actions;
            col_actions = node->col_actions;
        }

        void get_chance_actions(
            Types::Action row_action,
            Types::Action col_action,
            std::vector<typename Types::Obs> &chance_actions) const
        {
            const int row_idx = std::find(node->row_actions.begin(), node->row_actions.end(), row_action) - node->row_actions.begin();
            const int col_idx = std::find(node->col_actions.begin(), node->col_actions.end(), col_action) - node->col_actions.begin();
            const ChanceNode *chance_node = node->access(row_idx, col_idx);
            // TODO sort by prob for faster AlphaBeta
            for (auto kv : chance_node->stats.map)
            {
                chance_actions.push_back(kv.first);
            }
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action)
        {
            std::exception("Mapped State must specify chance action");
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action,
            Types::Obs chance_action)
        {
            const int row_idx = std::find(node->row_actions.begin(), node->row_actions.end(), row_action) - node->row_actions.begin();
            const int col_idx = std::find(node->col_actions.begin(), node->col_actions.end(), col_action) - node->col_actions.begin();
            const ChanceNode *chance_node = node->access(row_idx, col_idx);
            const ChanceData &chance_data = chance_node->stats.map.at(chance_action);
            // typename Types::PRNG device{chance_data.seed};
            Types::State::randomize_transition(chance_data.seed);
            Types::State::apply_actions(row_action, col_action);
            node = chance_node->access(Types::State::get_obs());
            assert(chance_action == Types::State::get_obs());
            this->prob = typename Types::Prob{
                typename Types::Q{static_cast<int>(chance_data.count), static_cast<int>(chance_node->stats.count)}};
            this->row_actions = node->row_actions;
            this->col_actions = node->col_actions;
        }

        bool is_terminal() const
        {
            return node->is_terminal();
        }

        Types::Value get_payoff() const
        {
            typename Types::State state_{*this};
            typename Types::Model model_{model};
            typename Types::Search search_{search};
            typename Types::MatrixNode root{};
            typename Types::PRNG device{};
            state_.get_actions();
            search_.run_for_iterations(iterations, device, state_, model_, root);
            typename Types::Value value{};
            search_.get_empirical_value(root.stats, value);
            // std::cout << value << std::endl;
            return value;
        }
    };
};