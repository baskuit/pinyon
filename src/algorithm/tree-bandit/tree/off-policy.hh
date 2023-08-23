#pragma once

#include <types/types.hh>
#include <tree/tree.hh>
#include <algorithm/algorithm.hh>

/*

TODO still no chance node updates (does any algo actually NEED this?)

*/

template <
    CONCEPT(IsBanditAlgorithmTypes, Types),
    template <typename...> typename NodePair = DefaultNodes,
    bool return_if_expand = true> // when would this be not true?

struct OffPolicy : Types
{
    struct MatrixStats : Types::MatrixStats
    {
        bool properly_expanded;
    };
    struct ChanceStats : Types::ChanceStats
    {
    };
    using MatrixNode = NodePair<Types, MatrixStats, ChanceStats>::MatrixNode;
    using ChanceNode = NodePair<Types, MatrixStats, ChanceStats>::ChanceNode;

    struct Frame
    {
        Frame(MatrixNode *matrix_node) : matrix_node{matrix_node} {}

        Types::Outcome outcome;
        MatrixNode *matrix_node;
    };

    using Trajectory = std::vector<Frame>;

    class Search : public Types::BanditAlgorithm
    {
    public:
        void run(
            const size_t learner_iterations,
            const size_t actor_iterations_per,
            Types::PRNG &device,
            const std::vector<typename Types::State> &states,
            Types::Model &model,
            std::vector<MatrixNode> &matrix_nodes)
        {
            // Perform batched inference on all trees
            // grab `actor` many samples, inference, update
            // do this `leaner` many times

            for (auto matrix_node : matrix_nodes)
            {
                // this->initialize_stats(learner_iterations * actor_iterations_per, states[0], model, matrix_node);
            } // currently don't call i_s anywhere even though its defined for bandits

            typename Types::ModelBatchInput input{};   // vector of states - Tensor
            typename Types::ModelBatchOutput output{}; // vector of inferences/single output - Tensor
            std::vector<Trajectory> trajectories{};

            for (int learner_iteration = 0; learner_iteration < learner_iterations; ++learner_iteration)
            {
                trajectories.clear();
                get_trajectories(trajectories, input,
                                 // set of paths to leaf nodes
                                 // all nodes on path must be updated for each leaf
                                 //
                                 actor_iterations_per, device, states, model, matrix_nodes);
                // populate trajectories vector and batch input

                model.inference(input, output);

                update_using_trajectories(trajectories, output);
                //
            }
        }

        void get_trajectories(
            // output parameters
            std::vector<Trajectory> &trajectories,
            Types::ModelBatchInput &input,
            // normal tree bandit params
            size_t actor_iterations_per,
            Types::PRNG &device,
            const std::vector<typename Types::State> &states,
            Types::Model &model,
            std::vector<MatrixNode> &matrix_nodes)
        {
            // One inference step
            for (int index = 0; index < matrix_nodes.size(); ++index)
            {
                MatrixNode *matrix_node = &matrix_nodes[index];
                const typename Types::State &state = states[index];
                // state, matrix node pair

                for (size_t actor_iteration = 0; actor_iteration < actor_iterations_per; ++actor_iteration)
                {
                    trajectories.emplace_back();
                    Trajectory &trajectory = trajectories.back();
                    // trajectory is a flat vector over matrix nodes and input iterations per

                    auto state_copy = state;
                    get_trajectory(trajectory,
                                   device, state_copy, model, matrix_node);
                    // populate vector of frames, rollout state to leaf node

                    if (!state_copy.is_terminal())
                    {
                        model.add_to_batch_input(state_copy, input);
                    }
                    // then add leaf state to inference pile
                }
            }
        }

        void update_using_trajectories(
            std::vector<Trajectory> &trajectories,
            Types::ModelBatchOutput &model_batch_output)
        {
            int index = 0;
            for (Trajectory &trajectory : trajectories)
            {
                Frame &leaf_frame = trajectory.front();
                MatrixStats &leaf_stats = leaf_frame.matrix_node->stats;

                typename Types::Value leaf_value;
                if (leaf_frame.matrix_node->is_terminal())
                {
                    leaf_value = leaf_frame.outcome.value;
                }
                else
                {
                    typename Types::ModelOutput model_output = model_batch_output[index++];
                    leaf_value = model_output.value;
                    if (!leaf_stats.properly_expanded)
                    {
                        this->post_expand(model_output, leaf_stats);
                        leaf_stats.properly_expanded = true;
                    }
                }

                for (Frame &frame : trajectory)
                {
                    frame.outcome.value = leaf_value;
                    this->update_matrix_stats_offpolicy(
                        frame.matrix_node->stats,
                        frame.outcome);
                }
            }
        }

        void get_trajectory(
            // output
            Trajectory &trajectory,
            // normal run_iteration args
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            MatrixNode *matrix_node)
        {
            Frame frame{matrix_node};

            if (state.is_terminal())
            {
                frame.outcome.value = state.payoff;
                matrix_node->set_terminal();
            }
            else
            {
                if (!matrix_node->is_expanded())
                {
                    matrix_node->expand(state);
                    matrix_node->stats.properly_expanded = false;
                    this->pre_expand(state, matrix_node->stats);
                }
                else
                {
                    state.get_actions();
                    if (matrix_node->stats.properly_expanded)
                    {
                        this->select(device, matrix_node->stats, frame.outcome);
                    }
                    else
                    {
                        const int rows = state.row_actions.size();
                        const int cols = state.col_actions.size();
                        frame.outcome.row_idx = device.random_int(rows);
                        frame.outcome.col_idx = device.random_int(cols);
                        frame.outcome.row_mu = typename Types::Q{1, rows};
                        frame.outcome.row_mu = typename Types::Q{1, cols};
                    }
                    state.apply_actions(
                        state.row_actions[frame.outcome.row_idx],
                        state.col_actions[frame.outcome.col_idx]);
                    ChanceNode *chance_node = matrix_node->access(frame.outcome.row_idx, frame.outcome.col_idx);
                    MatrixNode *matrix_node_next = chance_node->access(state.obs);

                    get_trajectory(trajectory, device, state, model, matrix_node_next);
                }
            }

            trajectory.push_back(frame);
        }

        // end algorithm
    };
};
