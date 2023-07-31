# Surskit

Surskit is a highly generic implementation of search for 'stochastic matrix tree games' in C++.

# Warning:

Surskit is in the final stages of a rewrite and release as version 0.1.0. The most current changes are in the *pruning* branch.

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
