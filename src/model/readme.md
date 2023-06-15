# DoubleOracleModel

Currently the only Models supported by Surskit are double oracle models, those that return a value and policy estimate for both players.

A value estimate is required by the TreeBandit algorithms. When a leaf node is expanded at the end of the forward phase, the value of its accompanying state is backpropogated to update the matrix/chance nodes leading up to the leaf node.

On the other hand, a policy estimate is not required by TreeBandit and is only used for some bandit algorithms. If a model does not typically supply a policy estimate (e.g. monte carlo), then the uniform policy is used as a stand-in.

## MonteCarloModel

This model applies the standard monte-carlo rollout method of value estimatation. It is a **universal model** that can be applied to any `PerfectInfoState`.

There are no limits on the length of a rollout. If a state (read: battle) become corrupted, typically by improper initialization or commiting invalid actions, it is possible that the corrupted state won't break the `get_actions`, `apply_actions` methods but will never reach a terminal state. This causes the monte carlo inference to loop indefinitely.

Monte Carlo inference is very weak. It is outperformed by any decent heuristic model. It's strength is also highly dependent on "depth-to-terminality"; The farther away a game state is from conclusion, the weaker this model's inference is. The same can be said for its computational performance as well. 

## Libtorch



## WrappedSearch