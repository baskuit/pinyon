#include <state/state.hh>

#include <tree/tree-debug.hh>

#include <memory>
#include <unordered_map>

template <
    typename Types,
    bool renormalize = false>
struct MappedState
{

    struct MatrixStats
    {
    };

    struct ChanceData
    {
        size_t count{};
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
                    typename Types::PRNG fixed_device{seed};
                    state_copy.randomize_transition(fixed_device);
                    state_copy.apply_actions(
                        matrix_node->row_actions[row_idx],
                        matrix_node->col_actions[col_idx]);

                    MatrixNode *matrix_node_next = chance_node->access(state_copy.get_obs());
                    auto &chance_data = chance_node->stats.map[state_copy.get_obs()];

                    if (chance_data.count == 0)
                    {
                        chance_data.prob = state_copy.prob;
                        total_prob += state_copy.prob;
                        chance_data.seed = seed;
                        run(
                            depth - 1,
                            tries,
                            device,
                            state_copy,
                            matrix_node_next);
                    }
                    ++chance_data.count;
                }
            }
        }
    }

    /*

    Expands a shared tree around the input state by brute-forcing chance nodes.
    This tree only stores the witnessed chance actions and their emprical probs.

    Much like `FullTraversal`, this is exactly a regular Types::State but with the shared data as well.
    When we  call `apply_actions` and transition to the of the shared tree, we mark as terminal.
    No search is done at that moment, but calling `get_payoff` will now perform a search with a disposable root node,
    and the model and search objects that were copied during initialization

    */

    class State : public Types::State
    {
    public:
        const MatrixNode
            *node;
        std::shared_ptr<const MatrixNode>
            explored_tree;
        std::shared_ptr<typename Types::Model>
            shared_model;
        std::shared_ptr<typename Types::Search>
            shared_search;
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
              shared_model{std::make_shared<typename Types::Model>(model)},
              shared_search{std::make_shared<typename Types::Search>(search)}
        {
            auto temp_tree = std::make_shared<MatrixNode>();
            node = temp_tree.get();
            run(depth, tries, device, state, temp_tree.get());
            explored_tree = temp_tree;
        }

        void get_chance_actions(
            Types::Action row_action,
            Types::Action col_action,
            std::vector<typename Types::Obs> &chance_action)
        {
            const int row_idx = std::find(node->row_actions.begin(), node->row_actions.end(), row_action) - node->row_actions.begin();
            const int col_idx = std::find(node->col_actions.begin(), node->col_actions.end(), col_action) - node->col_actions.begin();
            ChanceNode *chance_node = node->access(row_idx, col_idx);
            chance_node->map.keys();
            // TODO add keys to vector
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action,
            Types::Obs chance_action)
        {
            const int row_idx = std::find(node->row_actions.begin(), node->row_actions.end(), row_action) - node->row_actions.begin();
            const int col_idx = std::find(node->col_actions.begin(), node->col_actions.end(), col_action) - node->col_actions.begin();
            ChanceNode *chance_node = node->access(row_idx, col_idx);
            ChanceData &chance_data = chance_node->map[chance_action];
            typename Types::PRNG device = chance_data.fixed_device;
            Types::State::randomize_transition(device);
            Types::State::apply_actions(row_action, col_action);
            node = chance_node->access(Types::State::get_obs());
            assert(chance_action == Types::State::get_obs());
        }

        bool is_terminal() const
        {
            return node->is_terminal();
        }

        Types::Value get_payoff()
        {
            typename Types::MatrixNode root{};
            typename Types::PRNG device{};
            shared_search->run_for_iterations(iterations, device, *this, *shared_model, root);
            typename Types::Value value{};
            shared_search->get_empirical_value(value);
            return value;
        }
    };
};