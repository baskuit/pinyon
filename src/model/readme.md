
# Model
Models, like objects in the other categories, are assumed to be copy constructable. Neural network based models use shared pointers to the network, and heuristic and test models should have little to no data to copy.
Borrowing from machine learning, the process of estimating information about a model is called 'inference'.
A model must provide a value estimate in its inference because it is difficult to image a general search procedure that does not use this quantity.
### Symmetry
Models make the usual assumption that there is a fixed 'row' and 'column' player, but neither perspective is privileged. Since we also assume perfect information, there is no reason that a model should provide information for one player and not the other.
Therefore the value estimate of a model applies to both players. More precisely, the estimate is of `Value` type, with methods `get_row_payoff()` and `get_col_payoff()` returning a `Real` type.
### Policy
Not all search algorithms require a policy estimate for the players, and many model implementations do not provide one during inference

### Const Correctness
A model's inference is not assumed to treat the model or the input as const. This is because the multithreaded inference wrapper for Libtorch models has synchronization mechanisms that are modified when inference is called. Input is not assumed to be constant since the monte carlo model type as it's `ModelInput`

### State, Input, and Output
In order to fit monte carlo, heuristic, and Libtorch models 

### Batched

### Arena and SearchModel
There is a utility called Arena that evaluates the strength of a fixed pool of 'agents' by having two agents play out games from the beginning and returning the average payoff for each agent.
These 'agents' are intended to be bandit algorithms that a run for some amount of iterations/time, as a way to evaluate the strength of different algorithms. attached models, and hyper parameters.
However, we might also want to treat the unrefined (by search) policy output of a model as an agent too. Examples include the empty model, with uniform policy output, and the solved
Thus the policy model is taken as the 

# Concepts/Interface
```cpp
{
    output.value
} -> std::same_as<typename Types::Value &>;
```
The value estimate is a public member of the `ModelOutput` type.
```cpp
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

This model applies the standard Monte-Carlo rollout method of value estimation. It is a **universal model** that can be applied to any `PerfectInfoState`. For this reason the Monte-Carlo type list is a template that accepts the type list with a state.

There are no limits on the length of a rollout. If a state (read: battle) becomes corrupted, typically by improper initialization or committing invalid actions, then it is possible that the corrupted state won't throw an exception or cause a run-time error, but it will also never reach terminality. This causes the Monte Carlo inference to hang indefinitely.

Monte Carlo inference is very weak. It is outperformed by any decent heuristic model. It's strength is also highly dependent on "depth-to-terminal"; The farther away a game state is from conclusion, the weaker this model's inference is. The same can be said for its computational performance as well for obvious reasons.

## SolvedModel
This model assumes the underlying state type is a **solved state** and it simply returns the Nash equilibirum strategies and payoffs as its policy and value estimates. It has the same ancillary input and output types as MonteCarloModel since it is also universal, except that it must also provide a policy estimate in its output.

## SearchModel
The bandit algorithms are expected to have methods `get_empirical_strategies`, `get_emprical_values`, and these can be treated as policy and value estimates

One immediate application of this is for the FullTraversal and AlphaBeta solvers, where we can use this 'wrapped search' as the model. If these solvers are run with a finite max_depth, then this model will perform a normal bandit search at the leaf nodes of the sub-tree. 
This creates a 'hybrid' search algorithm that could reap the rewards of both solving and tree bandit search styles. 

## EmptyModel
This model gives $\frac{1}{2}, \frac{1}{2}$ as the value estimate and the uniform distribution over the actions as the policy estimate. It is used for benchmarking   

## Libtorch
> This section was written after completion of the multi-threaded inference wrapper but before integration of the wrapper with Surskit

The fastest way to use a neural network depends on its size. Small neural networks can work very well on modern CPUs, but larger networks run fastest on GPU. There is however a high cost to sending data between the CPU and GPU, so the only way to realize this performance benefit with large networks is to do inference in batches.

This requires a change in the search paradigm as well, and that is implemented in the `OffPolicy` tree algorithm. Details are discussed here [TODO relative link]



