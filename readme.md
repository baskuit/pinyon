# Surskit
![node](surskit.png?raw=true)

Surskit is a meta-algorithm implementation for stochastic matrix games using templates in C++.
It is a generalization of "growing tree" algorithms such as [SM-MCTS](https://arxiv.org/abs/1804.09045) and [MatrixUCB](https://arxiv.org/abs/2006.05145).

# Setup

    git clone --recurse-submodules https://www.github.com/baskuit/

# Types
Any game can be integrated with Surskit by wrapping it as a subclass of `State`. This only requires that you implement types for `Actions` and `Hash` and the MDP methods for stepping and getting legal actions:

`TransitionData transition (Action action0, Action action1)`
`PairActions actions ()`

A more detailed explanation of the type system is [here](https://github.com/baskuit/surskit/blob/master/src/readme.md).

## Gambit

**[Gambit](https://github.com/gambitproject/gambit)** is an open-source collection of tools for doing computation in game theory. 
The computation of Nash Equilibrium strategies is necessary for the MatrixUCB algorithm.