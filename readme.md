

# Pinyon

Pinyon is a high performance and general purpose library for research and development of search and solving algorithms for perfect-info, simultaneous move, stochastic games.
The library unifies a collection of environments, models, and algorithms with a consistent interface that is designed to make working with library as simple as possible for new C++ developers. It is intended for game enthusiasts first and developers second. The documentation does not assume advanced knowledge of C++ or Libtorch.

## Intended Use

This library is really attempt at reconciling two disparate realities:

* Pokemon is the highest grossing intellectual property of all time with over 20 years of competitive play in officially sanctioned and community organized formats.

* There are no agents that have been shown to be as strong as the best human players. In fact, most of the milestone developments of machine learning in other domains: (situational) tactical superiority, sound analysis, and computationally feasible search, have yet to be replicated here.

# Scope

As there is scarce publicly available work, this library attempts to attack machine learning in pokemon incrementally. By far the most important consequence of this attitude is information.

### Imperfect Info

The species of pokemon on a team, and each pokemon's moves, item, ability, etc are not known at the start of a game in any popular format. The management of this info is essential to high level play.

However my survey of methods in imperfect info games suggests that domain is significanltly more difficult and expensive to explore. Thus we make the following assumption that has natural justification

### Perfect Info

The hidden information of either player is only ever *revealed* during the course of a game, never renewed or augmented.
This means that games are always approaching a condition of perfect information. In practice, it is somewhat common for game states to become perfect information in practice. There are many ways that perfect info search can be (unsoundly) applied to imperfect info contexts and these are left to the user.

### Simulator
Pinyon has already been successfully applied to [engine](https://github.com/pkmn/engine), which is a high performance simulator for RBY written in Zig with a C interface. Any game which has a C/C++ interface can be wrapped and connected to Pinyon's utilities. 

### Performance

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

# Contributing

The best way to help is to use the library to test your own ideas. The interface is designed so that creating a custom search algorithm

That being said:
* Basic CUDA Backend
* 

# Release Checklist

The project is due for its first beta release. Only a few things need to be hammered out.

* Finishing touches on documentation
* Finish libtroch multithreaded wrapper
* Write up some **tests**
    * Tree structure - iterations = matrix node count; 
    * Soundness or **all** bandit search for calcing expl
    * Soundness of solver
    * Find old comments about ctests passing and see what you had back then

* Write up some **benchmarking utilities
    * Possibly include a way to benchmark retroactively
