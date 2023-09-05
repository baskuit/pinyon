
# Pinyon

Pinyon is a high performance and general purpose library for research and development of search and solving algorithms for perfect-info, simultaneous move, stochastic games.
The library unifies a collection of environments, models, and algorithms with a consistent interface that is designed to make working with library as simple as possible for new C++ developers.

## Intended Use

This library is really attempt at reconciling two disparate realities:

* Pokemon is the highest grossing intellectual property of all time with over 20 years of competitive play in community organized and later officially sanctioned formats.

* There are no agents that have been shown to be as strong as the best human players. In fact, most of the milestone developments of machine learning in other domains: (situational) tactical superiority, sound analysis, and computationally feasible search, have yet to be replicated.

# Scope

As there is scarce publicly available work, this library attempts to attack machine learning in Pokemon incrementally. By far the most important consequence of this attitude is the assumption about hidden information.

### Imperfect Info
The species of Pokemon on a team, and each Pokemon's moves, item, ability, etc are not known at the start of a game in any popular format. The management of this info is essential to high level play.

However my survey of methods in imperfect info games suggests that the imperfect info regime is expensive to explore. Thus we make the following assumption that has natural justification:

### Perfect Info
The hidden information of either player is only ever *revealed* during the course of a game, never renewed or augmented.
This means that games are always approaching a condition of perfect information. In practice, it is not uncommon for game states to become essentially perfect information. Additionally, there are many ways that perfect info search can be (unsoundly) applied to imperfect info contexts. These approaches are left to the user to implement and explore.

### Simulator
Pinyon has already been successfully applied to [engine](https://github.com/pkmn/engine), which is a high performance simulator written in Zig with a C interface. Any game which has a C/C++ interface can be wrapped and connected to Pinyon's utilities. 

### Performance
There are several existing libraries that aim to make planning and reinforcement learning accessible to non-technical users in a variety of games and environments:
* Open AI - [Gym](https://github.com/openai/gym)
* suragnair - [alpha-zero-general](https://github.com/suragnair/alpha-zero-general)

In order appeal to as many users as possible, these libraries expose their API via Python. With the mature projects like Gym, much of the functions are implemented in a performant language. 
This is mostly moot, because search and RL are both bottle-necked by the environment and it `step` functions.
**This is where Pinyon excels**
Without the overhead of a Python   environment, users can write code that is truly high performance.
Slow libraries do not present a realistic chance for users to write a strong search or train an agent on anything other than toy games. 

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
    * ~~Soundness of solver~~
    * ~~Find old comments about ctests passing and see what you had back then~~

* Write up some **benchmarking utilities
    * Possibly include a way to benchmark retroactively
