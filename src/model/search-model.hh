#pragma once

#include <model/model.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

/*

Model + Search wrapped as a new model.
One possible application is using this with a Solver with limited depth,
so that inference at the leafs is stronger than a normal model

*/

enum SearchModes
{
    TIME,
    PLAYOUTS
};

template <CONCEPT(IsSearchTypes, Types), bool HasPolicy = false>
struct TreeBanditSearchModel : Types::TypeList
{
    // onle way to recover any types object is via Types::TypeList.
    // so State = is rebuilding the type list, in this case TreeBanditSearchModel isStateTypes.
    using State = typename Types::State;
    using ModelInput = typename Types::State;
    struct ModelOutput
    {
        typename Types::Value value;
        typename Types::VectorReal row_policy, col_policy;
    };
    using ModelBatchInput = std::vector<typename Types::State>;
    using ModelBatchOutput = std::vector<typename Types::ModelOutput>;

    class Model
    {
    public:
        const size_t iterations;
        typename Types::PRNG device;
        typename Types::Model model;
        typename Types::Search session;
        const bool use_ms = false;

        Model(
            const size_t iterations,
            const Types::PRNG &device,
            const Types::Model &model,
            const Types::Search &session)
            : iterations{iterations}, device{device}, model{model}, session{session}
        {
        }

        void get_input(
            const State &state,
            ModelInput &input) const
        {
            input = state;
        }

        void get_batch_input(
            const std::vector<typename Types::State> &states,
            ModelBatchInput &inputs) const
        {
            inputs = states;
        }

        void inference(
            ModelInput &input,
            ModelOutput &output)
        {
            typename Types::MatrixNode root;
            if (use_ms == true)
            {
                session.run(iterations, device, input, model, root);
            }
            else
            {
                session.run_for_iterations(iterations, device, input, model, root);
            }
            session.get_empirical_strategies(root.stats, output.row_policy, output.col_policy);
            session.get_empirical_value(root.stats, output.value);
        }

        void inference(
            ModelBatchInput &inputs,
            ModelBatchOutput &outputs) const
        {
            outputs.resize(inputs.size());
            for (size_t i = 0; i < inputs.size(); ++i)
            {
                inference(inputs[i], outputs[i]);
            }
        }

        void add_to_batch_input(
            const Types::State &state,
            ModelBatchInput &input) const
        {
            input.push_back(state);
        }
    };
};
