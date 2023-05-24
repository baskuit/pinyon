These algorithms solve for Nash equilibrium payoffs and/or strategies on a (sub)tree

# `FullTraversal` 

expands the entire game tree with a single recursive function call. 

* It assumes the game is 1-sum (TODO: Consider adding MIN_PAYOFF, IS_CONSTANT_SUM, etc to States)

* The payoffs and strategies are stored in the matrix stats.

* State of requirement for the `TraversedState` wrapper. This is simply a `SolvedState` (it also satisfies conditions of a `ChanceState` but I'm not allowing multiple inheritance, and being solved is much less common.)

 # `AlphaBeta` 

is an implementation of Branislav Boˇsansk ́y, et al. for stochastic games

* The code and annotations reflect the paper's pseudo-code

* TODO: The MinVal, MaxVal put bounds on a chance node's value (the u_i,j of the paper) that tighten as the explored probability reaches 1. It is straightforward to check for this as you iterate through the chance node's branches, to terminate calculation early if the chance node can be pruned.

* Line 7 "p'_i,j" might be the wrong expression? TODO

* Serialized AlphaBeta is currently not implemented, not a priority.

* Expands the tree up to `depth=-1`, meaning infinite depth by default. The payoffs for nodes that are forced to be terminal (`depth=d` for some `d >= 0`) are given by `Model<>::get_inference()`, where `Model<>` derives from `DoubleOracleModel<>`.
