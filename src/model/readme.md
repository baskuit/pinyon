
# Model
Models are copy constructable. Neural network based models use shared pointers to the network, and any other model should have very relatively little data
### ValueModel vs DoubleOracleModel

### State, Input, and Output

### Batched

### Arena

# Concepts/Interface
```cpp
{
    output.value
} -> std::same_as<typename Types::Value &>;
{
    model.inference(
        input,
        output)
} -> std::same_as<void>;
{
    model.get_input(
        state,
        input)
} -> std::same_as<void>;
```
```cpp
{
    model.inference(model_batch_input, model_batch_output)
} -> std::same_as<void>;
{
    model.add_to_batch_input(state, model_batch_input)
} -> std::same_as<void>;
{
    model_batch_input[0]
} -> std::same_as<typename Types::ModelInput &>;
{
    model_batch_output[0]
} -> std::same_as<typename Types::ModelOutput &>;
```
```cpp
{
    output.row_policy
} -> std::same_as<typename Types::VectorReal &>;
{
    output.col_policy
} -> std::same_as<typename Types::VectorReal &>;
```

# Implementations

## MonteCarloModel

This model applies the standard Monte-Carlo rollout method of value estimation. It is a **universal model** that can be applied to any `PerfectInfoState`.

There are no limits on the length of a rollout. If a state (read: battle) becomes corrupted, typically by improper initialization or committing invalid actions, then it is possible that the corrupted state won't break the `get_actions`, `apply_actions` methods but will also never reach terminality. This causes the Monte Carlo inference to loop indefinitely.

Monte Carlo inference is very weak. It is outperformed by any decent heuristic model. It's strength is also highly dependent on "depth-to-terminality"; The farther away a game state is from conclusion, the weaker this model's inference is. The same can be said for its computational performance as well for obvious reasons.

## Libtorch

> This section was written after completion of the multi-threaded inference wrapper but before integration of the wrapper with Surskit

The fastest way to use a neural network depends on its size. Small neural networks can work very well on modern CPUs, but larger networks run fastest on GPU. There is however a high cost to sending data between the CPU and GPU, so the only way to realize this performance benefit with large networks is to do inference in batches.

This requires a change in the search paradigm as well, and that is implemented in the `OffPolicy` tree algorithm. Details are discussed here [TODO relative link]

## WrappedSearch

The bandit algorithms are expected to have methods `get_empirical_strategies`, `get_emprical_values`, and these can be treated as the policy and value output of a model, after some number of search iterations or course.

One immediate application of this is for the solver, where we can use this 'wrapped search' as the model. When the tree expanded to its maximum depth, this model's inference is called, which is the same as running a quick search instead.
