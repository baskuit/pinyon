# Surskit

Surskit is a highly generic implementation of search for 'stochastic matrix tree games' in C++.
At minimum, the user can wrap their specific game as a `DefaultState` and the provided implementations of [SM-MCTS](https://arxiv.org/abs/1804.09045) and [MatrixUCB](https://arxiv.org/abs/2006.05145), in conjunction with monte-carlo value estimation, will provide convergence guatantees. A compiled libtorch model or heuristic evaluation may be substituted all the same by wrapping it as a `DoubleOracleModel`.
If the user would like to create new algorithms, then they may be aided by the implementation pattern of Surskit that provides an inheritance framework for additional classes and a type management system that standardizes syntax. 
A more detailed explanation of the implementation is [here](https://github.com/baskuit/surskit/blob/master/src/readme.md).

## Gambit

**[Gambit](https://github.com/gambitproject/gambit)** is an open-source collection of tools for doing computation in game theory. 
The fast computation of Nash Equilibrium strategies is necessary for the MatrixUCB algorithm.

## Status
The code is largely untested and unbenchmarked. Next steps are to correct this and then to create tools for evaluating the relative performance of search schemes and generating replay data for model training and fine-tuning.
Otherwise, much of my development on Surskit will be used for my pet case, and thus likely won't be added to the repo.
