#pragma once

#include <types/types.hh>
#include <tree/tree.hh>
#include <algorithm/algorithm.hh>

/*

TODO still no chance node updates (does any algo actually NEED this?)

*/

/*

we do not keep track of the rolled out states outside of the inner most get_trajectory (run_iteration) function
therefore to expand we need to do some part of it when we have access to the state
and the other part later when we have access to the inference

but we cannot call the default select method on a state-expanded node, in general.
and it should only be the uniform policy anyway, so we need a way to distinguish
selection nodes from passable nodes. is_expanded is taken to mean passable, and properly_expanded
in the stats is used for selectable. 2 bools for 3 options


*/

template <
    CONCEPT(IsBanditAlgorithmTypes, Types),
    template <typename...> typename NodePair = DefaultNodes,
    bool return_if_expand = true> // when would this be not true?

struct OffPolicy : Types
{
    struct MatrixStats : Types::MatrixStats
    {
        bool properly_expanded = false;
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
        using Types::BanditAlgorithm::BanditAlgorithm;

        Search(const Types::BanditAlgorithm &base) : Types::BanditAlgorithm{base} {}

        friend std::ostream &operator<<(std::ostream &os, const Search &search)
        {
            os << "OffPolicy - ";
            os << static_cast<typename Types::BanditAlgorithm>(search);
            os << " - " << NodePair<Types, typename Types::MatrixStats, typename Types::ChanceStats>{};
            return os;
        }

        // void run(
        //     const size_t learner_iterations,
        //     const size_t actor_iterations_per,
        //     Types::PRNG &device,
        //     const std::vector<typename Types::State> &states,
        //     Types::Model &model,
        //     std::vector<MatrixNode *> &matrix_nodes)
        // {

        // }

        size_t run(
            const size_t duration_ms,
            const size_t actor_iterations_per,
            Types::PRNG &device,
            const std::vector<typename Types::State> &states,
            Types::Model &model,
            std::vector<MatrixNode> &matrix_nodes)
        {
            // Perform batched inference on all trees
            // grab `actor` many samples, inference, update
            // do this `leaner` many times

            for (auto &matrix_node : matrix_nodes)
            {
                // this->initialize_stats(learner_iterations * actor_iterations_per, states[0], model, matrix_node);
            } // currently don't call i_s anywhere even though its defined for bandits

            typename Types::ModelBatchInput model_batch_input{};   // vector of states - Tensor
            typename Types::ModelBatchOutput model_batch_output{}; // vector of inferences/single output - Tensor
            std::vector<Trajectory> trajectories{};

            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            size_t iterations = 0;
            for (; duration.count() < duration_ms; ++iterations)
            {
                trajectories.clear();
                get_trajectories(trajectories, model_batch_input,
                                 // set of paths to leaf nodes
                                 // all nodes on path must be updated for each leaf
                                 //
                                 actor_iterations_per, device, states, model, matrix_nodes);
                // populate trajectories vector and batch input

                model.inference(model_batch_input, model_batch_output);

                update_using_trajectories(trajectories, model_batch_output);
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

                //
            }
            return iterations;
        }

        size_t run_for_iterations(
            const size_t learner_iterations,
            const size_t actor_iterations_per,
            Types::PRNG &device,
            const std::vector<typename Types::State> &states,
            Types::Model &model,
            std::vector<MatrixNode> &matrix_nodes)
        {
            // for (auto &matrix_node : matrix_nodes)
            // {
            //     // this->initialize_stats(learner_iterations * actor_iterations_per, states[0], model, matrix_node);
            // } // currently don't call i_s anywhere even though its defined for bandits

            std::vector<Trajectory> trajectories{};

            auto start = std::chrono::high_resolution_clock::now();
            for (int learner_iteration = 0; learner_iteration < learner_iterations; ++learner_iteration)
            {
                // TODO All scrwed up. Need a way to convert tensor to model_output
                // probably needs helper class.... for logit masks n such
                typename Types::ModelBatchInput model_batch_input = model.get_random_input(0); // vector of states - Tensor

                trajectories.clear();
                get_trajectories(trajectories, model_batch_input,
                                 actor_iterations_per, device, states, model, matrix_nodes);
                // populate trajectories vector and batch input
                typename Types::ModelBatchOutput model_batch_output = model.get_random_output(0); // vector of inferences/single output - Tensor

                model.inference(model_batch_input, model_batch_output);

                update_using_trajectories(trajectories, model_batch_output);
                //
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return duration.count();
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
                        model.add_to_batch_input(std::move(state_copy), input);
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
                if (leaf_frame.matrix_node->is_terminal()) [[unlikely]]
                {
                    leaf_value = leaf_frame.outcome.value;
                }
                else [[likely]]
                {
                    typename Types::ModelOutput model_output{};
                    typename Types::Mask mask{};
                    model.get_output(model_output, model_batch_output, index, mask);
                    leaf_value = model_output.value;
                    if (!leaf_stats.properly_expanded)
                    {
                        this->expand_inference_part(leaf_stats, model_output);
                        leaf_stats.properly_expanded = true;
                    }
                }

                int frame_index = 0;
                for (Frame &frame : trajectory)
                {
                    if (frame_index == 0)
                    {
                        frame_index++;
                        continue;
                    } // skip last frame in trajectory

                    frame.outcome.value = leaf_value;
                    this->update_matrix_stats_offpolicy(
                        frame.matrix_node->stats,
                        frame.outcome);
                    // the difference is that the outcome row_mu entries reflect that of the 'actor'
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
                    const size_t rows = state.row_actions.size();
                    const size_t cols = state.col_actions.size();
                    matrix_node->expand(rows, cols);
                    this->expand_state_part(matrix_node->stats, rows, cols);
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
                        frame.outcome.col_mu = typename Types::Q{1, cols};
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
    };
};
