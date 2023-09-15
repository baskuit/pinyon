
# Pinyon
Pinyon is a high performance library for research and development of search and solving algorithms for perfect-info, simultaneous move, stochastic games.

# Overview
Most users have a specific game that they want to create a strong agent for.

The wrapper, together with universal Monte-Carlo estimation, will instantly have access to:
* Sound Search
Most tutorials on MCTS **will not work** in the simultaneous move regime. This is a huge barrier for entry into engine development. Pinyon provides algorithms that provably converge to Nash equilibrium.
* Multi-threaded search
Modern processors are multi-core and any search that does not exploit this will be substantially weaker. Multi-threading in C++ is daunting for most developers so there are two parallelized MCTS search variants that provide it for free. This will work for any game or algorithm.
* Alpha-Beta solving
Conventional depth-first search was revolutionized by Alpha-Beta, which cleverly prunes nodes it can prove don't affect the outcome of the search. This is the first public implementation of Alpha-Beta in the simultaneous regime based on research. The paper's algorithm is even improved to prune insignificant chance outcomes. 
* High performance
This library is much faster than peers because it's written entirely in a compiled language. It leverages template meta-progamming so that the generality and modularity produces **no run-time cost**.

These are features that provided by the library for free and without hassle.
If the user would like to tweak these they will find that **virtually all aspects of search can be changed**. Again, 

* Use your own models
If the user wants to develop or improve a value estimate other than Monte-Carlo then they simply need to wrap their 'Model' just as they did their game. Once compatible with Pinyon the model can be used instead for all the previous features.




* Tree structure
This is one of the simplest ways to squeeze more performance out of Pinyon. Search tree operations are most of the run-time and so Pinyon offers several different implementations

If the user has another model they've already written they can use that instead all the same. 

The user can tweak their code to maximize performance

* Many different kinds of tree structure
* Different kinds of primitives

The user can write new algorithms

* Creating a new bandit is done by implementing a few functions
* Totally interchangeable with old bandits
* Automatically compatible with multithreaded search


### Reinforcement Learning

### Library Utilities

* Powerful class of test games
* GMP support for precise numbers
* Fast NE solving

Most importantly, the sum total of all these optinos are contained as a type list.
This is the most important part of Pinyon. 


# Installation
This project makes heavy use of the `Concepts` feature of C++20. Furthermore, it occasionally uses `std::cartiesian_product` from C++23. Support for the latter is spotty, but GCC-13 will work
The [GNU Multiple Precision Arithmetic Library](https://gmplib.org/) is required for use in
```
sudo apt install libgmp3-dev
git clone --recurse-submodules https://github.com/baskuit/pinyon
cd pinyon
mkdir build
cd build
cmake ..
make
```
Note: make sure that the correct version of gcc/clang is being used. I recommend using VSCode to build this library. See the /src level readme [here](src/readme.md#LanguageandDevelopmentEnvironment)

# Documentation

The many facets of the library are well documented

Basic
Types
State
Model
Algorithm

Additionally the library cites its sources. The research papers used are all in
References


# Status
Done with perfect info stuff. Taking a break and looking for users. I made this library because I want to apply my expertise to model development in certain goalpost domains but I find training and tweaking to be very boring.
Later this year I will start work on the imperfect info regime.
