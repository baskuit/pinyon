# Surskit
![node](surskit.png?raw=true)

Surskit is a meta-algorithm implementation for stochastic matrix games using templates in C++.
It is a generalization of "growing tree" algorithms such as [SM-MCTS](https://arxiv.org/abs/1804.09045) and [MatrixUCB](https://arxiv.org/abs/2006.05145).

A more detailed explanation of the type system is [here](https://github.com/baskuit/surskit/blob/master/src/readme.md).

## Gambit

**[Gambit](https://github.com/gambitproject/gambit)** is an open-source collection of tools for doing computation in game theory. 
The computation of Nash Equilibrium strategies is necessary for the MatrixUCB algorithm.

## Status
The core of surskit, which is the MatrixUCB/Exp3p algorithm, is implemented with the new design pattern (including multithreaded).
Feature-wise the next step is Libtorch and replay generation. But I will likely spend some time with documentation and tests.
