#pragma once

#include <model/model.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

/*

Model + Search wrapped as a new model.
One possible application is using this with a Solver with limited depth,
so that inference at the leafs is stronger than a normal model

*/

template <IsSearchTypes Types, bool HasPolicy = false>
struct SearchModel : Types::TypeList
{
    // onle way to recover any types object is via Types::TypeList.
    // so State = is rebuilding the type list, in this case SearchModel isStateTypes.
    using State = typename Types::State;
    using ModelInput = typename Types::State;
    struct ModelOutput
    {
        typename Types::Value value;
    };
    using ModelBatchInput = std::vector<typename Types::State>;
    using ModelBatchOutput = std::vector<typename Types::ModelOutput>;

    class Model
    {
    public:
        const size_t iterations{1 << 10};
        typename Types::PRNG device{};
        typename Types::Model model{};
        typename Types::Search session{};

        Model() {}

        Model(const size_t iterations) : iterations{iterations}
        {
        }

        Model(
            const size_t iterations,
            const typename Types::PRNG &device,
            const typename Types::Model &model,
            const typename Types::Search &session) : iterations{iterations}, device{device}, model{model}, session{session}
        {
        }

        void get_input(
            const typename Types::State &state,
            typename Types::ModelInput &input)
        {
            input = state;
        }

        void get_batch_input(
            const std::vector<typename Types::State> &states,
            typename Types::ModelBatchInput &inputs)
        {
            inputs = states;
        }

        void get_inference(
            typename Types::ModelInput &input,
            typename Types::ModelOutput &output)
        {
            typename Types::MatrixNode root;
            session.run(iterations, device, input, model, root);
            session.get_empirical_strategies(root.stats, output.row_policy, output.col_policy);
            session.get_emprical_value(root.stats, output.value);
        }

        void get_inference(
            typename Types::ModelBatchInput &inputs,
            typename Types::ModelBatchOutput &outputs)
        {
            outputs.resize(inputs.size());
            for (size_t i = 0; i < inputs.size(); ++i)
            {
                get_inference(inputs[i], outputs[i]);
            }
        }

        void add_to_batch_input(
            typename Types::State &state,
            typename Types::ModelBatchInput &input)
        {
            input.push_back(state);
        }
    };
};

template <typename T, typename = void>
struct AssertModelDoesNotExist : std::true_type {};

template <typename T>
struct AssertModelDoesNotExist<T, std::void_t<typename T::Model>> : std::false_type {};


template <typename State, typename Types>
requires std::is_same_v<State, typename Types::State>
auto StateTypes (State state) -> Types {
    return {};
}

template <typename Types, typename OldTypes>
concept Foo = std::derived_from<Types, OldTypes> && AssertModelDoesNotExist<OldTypes>::value;


template <IsSearchTypes Types, IsStateTypes OldTypes>
// requires AssertModelDoesNotExist<OldTypes>::value && std::derived_from<Types, OldTypes>
requires Foo<Types, OldTypes>
auto SearchModel_(Types, OldTypes) -> SearchModel<Types>
{
    return SearchModel<Types>{};
}