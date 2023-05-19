# Surskit

Surskit is a highly generic implementation of search for 'stochastic matrix tree games' in C++.

At minimum, the user can wrap their specific game as a `DefaultState` and the provided implementations of [SM-MCTS](https://arxiv.org/abs/1804.09045) and [MatrixUCB](https://arxiv.org/abs/2006.05145) will provide convergence guarantees. 
A compiled libtorch model or heuristic evaluation may be substituted all the same by wrapping it as a `DoubleOracleModel`.

If the user would like to create new algorithms, then they may be aided by the implementation pattern of Surskit that provides an inheritance framework for additional classes and a type management system that standardizes syntax. 

A more detailed explanation of the implementation is [here](https://github.com/baskuit/surskit/blob/master/src/readme.md).

## Installation

    git clone --recurse-submodules https://github.com/baskuit/surskit
    cd surskit
    mkdir build
    cd build
    cmake ..
    make

Windows is currently not supported

## Gambit

**[Gambit](https://github.com/gambitproject/gambit)** is an open-source collection of tools for doing computation in game theory. 
The fast computation of Nash Equilibrium strategies is necessary for the MatrixUCB algorithm.

## Status
Master branch is likely not building right now but I'm refactoring using strong typing and doing some important design changes on the 'types' branch.
Integration with pkmn/engine is proven and this has been moved to 'Taurus' repo.
Once the refactor is done the next priority is *extensive* search parameter testing on random trees
After that, extending AlpheBeta functionality to general `State`s using brute force calculation of transitions.
Then I want to write a general batched inference wrapper around Libtorch models, and if that goes well, a Python binding to move testing and NN design to a more accessible platform.
