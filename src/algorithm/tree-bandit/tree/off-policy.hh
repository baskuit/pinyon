#pragma once

#include <types/types.hh>
#include <tree/tree.hh>
#include <algorithm/algorithm.hh>

/*

TODO still no chance node updates (does any algo actually NEED this?)

*/

template <
    class BanditAlgorithm,
    template <class> class MNode,
    template <class> class CNode,
    bool return_if_expand = true>
class OffPolicy : public BanditAlgorithm
{
public:
    struct Types : BanditAlgorithm::Types
    {
        using MatrixNode = MNode<OffPolicy>;
        using ChanceNode = CNode<OffPolicy>;
    };

    struct Frame
    {
        Types::Outcome outcome;
        MatrixNode<OffPolicy> *matrix_node;
        Frame(
            typename Types::Real outcome,
            MatrixNode<OffPolicy> *matrix_node) : outcome{outcome}, matrix_node{matrix_node} {}
        Frame() {}
    };
    using Trajectory = std::vector<Frame>;

    void run(
        const size_t learner_iterations,
        const size_t actor_iterations_per,
        typename Types::PRNG &device,
        std::vector<typename Types::State> &states,
        typename Types::Model &model,
        std::vector<MatrixNode<OffPolicy> *> &matrix_nodes)
    {

        // Perform batched inference on all trees
        // grab `actor` many samples, inference, update
        // do this `leaner` many times

        for (auto matrix_node : matrix_nodes)
        {
            // this->initialize_stats(learner_iterations * actor_iterations_per, states[0], model, matrix_node);
        }

        typename Types::ModelBatchInput input{};   // vector of states - Tensor
        typename Types::ModelBatchOutput output{}; // vector of inferences/single output - Tensor
        std::vector<Trajectory> trajectories{};

        for (int learner_iteration = 0; learner_iteration < learner_iterations; ++learner_iteration)
        {
            trajectories.clear();
            get_trajectories(trajectories, input,
                             actor_iterations_per, device, states, model, matrix_nodes);
            // populate trajectories vector and batch input

            model.get_inference(input, output);

            update_using_trajectories(trajectories, output);
        }
    }

    void get_trajectories(
        std::vector<Trajectory> &trajectories,
        typename Types::ModelBatchInput &input,
        size_t actor_iterations_per,
        typename Types::PRNG &device,
        std::vector<typename Types::State> &states,
        typename Types::Model &model,
        std::vector<MatrixNode<OffPolicy> *> &matrix_nodes)
    {

        // One inference step

        int state_index = 0;
        for (auto matrix_node : matrix_nodes)
        {
            auto &state = states[state_index];

            for (size_t actor_iteration = 0; actor_iteration < actor_iterations_per; ++actor_iteration)
            {
                trajectories.emplace_back();
                Trajectory &trajectory = trajectories.back();

                auto state_copy = state;
                get_trajectory(trajectory, device, state_copy, model, matrix_node);
                model.add_to_batch_input(state_copy, input); // push_back TODO
            }
            ++state_index;
        }
    }

    void update_using_trajectories(
        std::vector<Trajectory> &trajectories,
        typename Types::ModelBatchOutput &output)
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

                double learning_rate = (row_policy[outcome.row_idx] * col_policy[outcome.col_idx]) / (outcome.row_mu * outcome.col_mu);

                std::cout << learning_rate << " lr" << std::endl;

                this->update_matrix_stats(
                    matrix_node->stats,
                    outcome,
                    learning_rate);
            }
            ++index;
        }
    }

protected:
    void get_trajectory(
        Trajectory &trajectory,
        typename Types::PRNG &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<OffPolicy> *matrix_node)
    {
        if (!matrix_node->is_terminal)
        {
            if (matrix_node->is_expanded)
            {
                trajectory.emplace_back();
                auto &frame = trajectory.back();
                frame.matrix_node = matrix_node;

                typename Types::Outcome &outcome = frame.outcome;
                this->select(device, matrix_node->stats, outcome);

                typename Types::Action row_action = matrix_node->row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<OffPolicy> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<OffPolicy> *matrix_node_next = chance_node->access(state.obs);

                get_trajectory(trajectory, device, state, model, matrix_node_next);
                return;
            }
            else
            {
                state.get_actions();
                matrix_node->row_actions = state.row_actions;
                matrix_node->col_actions = state.col_actions;
                matrix_node->is_expanded = true;
                matrix_node->is_terminal = state.is_terminal;

                typename Types::ModelOutput inference;
                this->expand(state, matrix_node->stats, inference);
                return;
            }
        }
        else
        {
            return;
        }
    }
};