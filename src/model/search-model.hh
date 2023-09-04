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
    using State = typename Types::State; // DONT REMOVE
    // so State = is rebuilding the type list, in this case TreeBanditSearchModel isStateTypes.
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
        typename Types::Search search;
        const bool use_ms = false;

        Model(
            const size_t iterations,
            const Types::PRNG &device,
            const Types::Model &model,
            const Types::Search &search)
            : iterations{iterations}, device{device}, model{model}, search{search}
        {
        }

        void inference(
            Types::State &&input,
            ModelOutput &output)
        {
            typename Types::MatrixNode root;
            if (use_ms == true)
            {
                search.run(iterations, device, input, model, root);
            }
            else
            {
                search.run_for_iterations(iterations, device, input, model, root);
            }
            search.get_empirical_strategies(root.stats, output.row_policy, output.col_policy);
            search.get_empirical_value(root.stats, output.value);
        }

        void inference(
            ModelBatchInput &batch_input,
            ModelBatchOutput &batch_output)
        {
            batch_output.resize(batch_input.size());
            for (size_t i = 0; i < batch_input.size(); ++i)
            {
                inference(batch_input[i], batch_output[i]);
            }
        }

        void add_to_batch_input(
            Types::State &&state,
            ModelBatchInput &batch_input) const
        {
            batch_input.push_back(state);
        }
    };
};
