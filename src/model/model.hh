#pragma once

#include <state/state.hh>

#include <vector>

template <class _State>
class AbstractModel
{
public:
    struct Types : _State::Types
    {
        using State = _State;
    };
};

/*
Similar to `State`, in that virtually all models will be derived from it.
*/

template <class _State>

class DoubleOracleModel : public AbstractModel<_State>
{
    static_assert(std::derived_from<_State, State<typename _State::Types::TypeList>>);

public:
    struct ModelOutput;

    struct Types : AbstractModel<_State>::Types
    {
        using ModelOutput = DoubleOracleModel::ModelOutput;
    };

    struct ModelOutput
    {
        typename Types::Value value;
        typename Types::Strategy row_policy;
        typename Types::Strategy col_policy;
    };
};

/*
Universal model.
*/

template <class _State>
class MonteCarloModel : public DoubleOracleModel<_State>
{

public:
    struct Types : DoubleOracleModel<_State>::Types
    {
        using ModelBatchOutput = std::vector<typename Types::ModelOutput>;
        using ModelBatchInput = std::vector<_State>;
        using ModelInput = _State;
    };

    typename Types::PRNG device;

    MonteCarloModel(typename Types::PRNG &device) : device(device) {}

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
        const typename Types::Real row_uniform{1 / (typename Types::Real)input.row_actions.size()};
        output.row_policy.fill(input.row_actions.size(), row_uniform);
        const typename Types::Real col_uniform{1 / (typename Types::Real)input.col_actions.size()};
        output.col_policy.fill(input.col_actions.size(), col_uniform);

        rollout(input);
        output.value = input.payoff;
    }

    void get_inference(
        typename Types::ModelBatchInput &inputs,
        typename Types::ModelBatchOutput &outputs)
    {
        outputs.resize(inputs.size());
        // add empty structs
        for (int i = 0; i < inputs.size(); ++i) {
            auto &input = inputs[i];
            auto &output = outputs[i];
            get_inference(input, output);
        }
    }

    void add_to_batch_input (
        typename Types::State &state,
        typename Types::ModelBatchInput &input
    ) {
        input.push_back(state);
    }

protected:
    void rollout(_State &state)
    {
        // model inference in bandits happens in expand(), after get_actions is called
        while (!state.is_terminal)
        {
            const ActionIndex row_idx = device.random_int(state.row_actions.size());
            const ActionIndex col_idx = device.random_int(state.col_actions.size());
            const typename Types::Action row_action = state.row_actions[row_idx];
            const typename Types::Action col_action = state.col_actions[col_idx];
            state.apply_actions(row_action, col_action);
            state.get_actions();
        }
    }
};

template <class _State>
class EmptyModel : public DoubleOracleModel<_State>
{

public:
    struct Types : DoubleOracleModel<_State>::Types
    {
        using ModelBatchOutput = std::vector<typename Types::ModelOutput>;
        using ModelBatchInput = std::vector<_State>;
        using ModelInput = _State;
    };

    typename Types::PRNG device;

    EmptyModel(typename Types::PRNG &device) : device(device) {}

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
        output.value = typename Types::Value{.5, .5};
    }

    void get_inference(
        typename Types::ModelBatchInput &inputs,
        typename Types::ModelBatchOutput &outputs)
    {
        // TODO use transform
    }
};