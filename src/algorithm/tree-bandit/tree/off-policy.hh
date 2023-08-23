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
    using MatrixNode = NodePair<Types>::MatrixNode;
    using ChanceNode = NodePair<Types>::ChanceNode;

    struct Frame
    {
        Frame(MatrixNode *matrix_node) : matrix_node{matrix_node} {}
        Types::Outcome outcome; // row, col index; value; policy
        // need policy here for importance weight, or at least mu
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
            std::vector<MatrixNode *> &matrix_nodes)
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
            std::vector<MatrixNode *> &matrix_nodes)
        {
            // One inference step
            for (int index = 0; index < matrix_nodes.size(); ++index)
            {
                const MatrixNode *const matrix_node = matrix_nodes[index];
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
            Types::ModelBatchOutput &output)
        {
            int index = 0;
            for (Trajectory &trajectory : trajectories)
            {
                typename Types::ModelOutput &out = output[index];
                for (Frame &frame : trajectory)
                {
                    auto matrix_node = frame.matrix_node;
                    auto &outcome = frame.outcome;
                    outcome.value = out.value;

                    typename Types::VectorReal row_policy;
                    typename Types::VectorReal col_policy;

                    this->get_policy(matrix_node->stats, row_policy, col_policy);

                    const auto learning_rate = row_policy[outcome.row_idx] * col_policy[outcome.col_idx] / (outcome.row_mu * outcome.col_mu);

                    this->update_matrix_stats(
                        matrix_node->stats,
                        outcome,
                        learning_rate);
                }


                if (!trajectory.front().matrix_node->is_terminal()) {
                    ++index;
                }
            }
        }

    protected:
        void get_trajectory(
            // output
            Trajectory &trajectory,
            // normal run_iteration args
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            MatrixNode *matrix_node)
        {
            // trajectory.emplace_back(matrix_node);
            // Frame &frame = trajectory.back();
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
                    typename Types::ModelOutput model_output;
                    this->expand(state, matrix_node->stats, model_output);
                }
                else
                {
                    this->select(device, matrix_node->stats, frame.outcome);
                    state.get_actions();
                    state.apply_actions(
                        state.row_actions[frame.outcome.row_idx],
                        state.col_actions[frame.outcome.col_idx]
                    );

                    ChanceNode *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                    MatrixNode *matrix_node_next = chance_node->access(state.obs);

                    get_trajectory(trajectory, device, state, model, matrix_node_next);
                }
            }

            trajectory.push_back(frame);
        }

        // end algorithm
    };
};