
# DoubleOracleModel

Currently the only Models supported by Surskit are 'double oracle' models, those that return a value and policy estimate for both players.

A value estimate is required by the tree-bandit algorithms. When a leaf node is expanded at the end of the forward phase, the value of its accompanying state is back-propagated to update the matrix/chance nodes leading up to the leaf node.

On the other hand, a policy estimate is not required by tree-bandit search and is only used in some bandit algorithms. If a model does not typically supply a policy estimate (e.g. monte carlo), then the uniform policy is used as a stand-in.

### Interface

Any model derived from `DoubleOracleModel` is expected to define 4 new types to be added to the `Types` struct.

* `ModelInput`
* `ModelOutput`
* `ModelBatchInput`
* `ModelBatchOutput`

These types are used in the inference and helper methods below:

```cpp
    void get_input(
        const typename Types::State &state,
        typename Types::ModelInput &input);
        
    void get_batch_input(
        const std::vector<typename Types::State> &states,
        typename Types::ModelBatchInput &inputs);

    void add_to_batch_input (
        const typename Types::State &state,
        typename Types::ModelBatchInput &input);

    void get_inference(
        typename Types::ModelInput &input,
        typename Types::ModelOutput &output);

    void get_inference(
        typename Types::ModelBatchInput &inputs,
        typename Types::ModelBatchOutput &outputs);
```

* `get_input`
The pytorch models take a tensor as input, not the raw state itself.
* `get_batch_input`
Essentially the same method as above, but plural. The gist of what it does depends on the model. For example, the monte carlo model simply defines a vector of states as its input, and 

Notice the `const` qualifier on the state parameters. Indeed, providing expert knowledge should not alter a state.

The type `ModelOutput` is already defined in this abstract base class. It is simply a struct with the aforementioned value and policy pairs. It is defined here because the derived model classes below all agree on this definition.


## MonteCarloModel

This model applies the standard Monte-Carlo rollout method of value estimation. It is a **universal model** that can be applied to any `PerfectInfoState`.

There are no limits on the length of a rollout. If a state (read: battle) becomes corrupted, typically by improper initialization or committing invalid actions, then it is possible that the corrupted state won't break the `get_actions`, `apply_actions` methods but will also never reach terminality. This causes the Monte Carlo inference to loop indefinitely.

Monte Carlo inference is very weak. It is outperformed by any decent heuristic model. It's strength is also highly dependent on "depth-to-terminality"; The farther away a game state is from conclusion, the weaker this model's inference is. The same can be said for its computational performance as well for obvious reasons.

## Libtorch

The 



## WrappedSearch
