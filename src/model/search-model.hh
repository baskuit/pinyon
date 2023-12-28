#pragma once

#include <model/model.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

/*

Model + Search wrapped as a new model.
One possible application is using this with a Solver with limited depth,
so that inference at the leafs is stronger than a normal model

*/

namespace SearchModelDetail
{

    template <typename Types, bool use_policy>
    struct ModelOutputImpl;

    template <typename Types>
    struct ModelOutputImpl<Types, true>
    {
        typename Types::Value value;
        typename Types::VectorReal row_policy, col_policy;
    };

    template <typename Types>
    struct ModelOutputImpl<Types, false>
    {
        typename Types::Value value;
        typename Types::VectorReal row_policy, col_policy;
    };

};

template <CONCEPT(IsSearchTypes, Types), bool use_iterations = true, bool use_policy = true>
struct SearchModel : Types::TypeList
{
    using State = typename Types::State; // DONT REMOVE
    // so State = is rebuilding the type list, in this case SearchModel isStateTypes.

    using ModelOutput = SearchModelDetail::ModelOutputImpl<Types, use_policy>;

    using ModelBatchInput = std::vector<typename Types::State>;
    using ModelBatchOutput = std::vector<typename Types::ModelOutput>;

    class Model
    {
    public:
        // time parameter (ms), number of iterations, or depth (in the case of solvers)
        const size_t count;
        typename Types::PRNG device;
        typename Types::Model model;
        typename Types::Search search;

        Model(
            const size_t count,
            const Types::PRNG &device,
            const Types::Model &model,
            const Types::Search &search)
            : count{count}, device{device}, model{model}, search{search}
        {
        }

        void inference(
            Types::State &&state,
            ModelOutput &output)
        {
            typename Types::MatrixNode root;

            if constexpr (use_iterations == true)
            {
                search.run_for_iterations(count, device, state, model, root);
            }
            else
            {
                // as ms
                search.run(count, device, state, model, root);

            }

            if constexpr (use_policy)
            {
                search.get_empirical_strategies(root.stats, output.row_policy, output.col_policy);
            }
            
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
