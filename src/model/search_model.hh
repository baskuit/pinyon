#pragma once

#include <model/model.hh>
#include <tree/tree.hh>

template <class Algorithm>
class SearchModel : public DoubleOracleModel<typename Algorithm::Types::State>
{

    struct Types : DoubleOracleModel<typename Algorithm::Types::State>::Types
    {
        using ModelBatchOutput = std::vector<typename Types::ModelOutput>;
        using ModelBatchInput = std::vector<State>;
        using ModelInput = State;
    };

    const size_t iterations{1 << 10};
    typename Types::PRNG device{};
    typename Algorithm::Types::Model model{};
    Algorithm session{};

    SearchModel() {}

    SearchModel(const size_t iterations) : iterations{iterations}
    {
    }

    SearchModel(
        const size_t iterations,
        const typename Types::PRNG &device,
        const typename Algorithm::Types::Model model,
        const Algorithm &session) : iterations{iterations}, device{device}, model{model}, session{session}
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
        MatrixNode<Algorithm> root;
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