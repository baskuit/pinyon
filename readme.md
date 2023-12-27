
# Pinyon

Pinyon is a high performance library for research and development of search and solving algorithms for perfect-info, simultaneous move, stochastic games. The code emphasizes:

* Compile-Time

This library uses the template metaprogramming features of C++ to eliminate the cost of game/alorithm/etc agnosticism that other libraries like TODO cost. Pinyon is then fully capable of tasks like powering expensive reinforcement-learning projects or automated deep analysis.

* Fully Modular

The game, evaluation function, and search algorithm are liberally interchangible, exploiting the compile-time optimization. The interface was designed from the beginning to incorporate all major SOTA methods. Serveral other basic utilities are provided as well.

* Simple to Start With

Virtually any game can be augmented with a simple and fast monte-carlo search function, providing guarantees of convergence to Nash Equilibrium.

* Highly Abstractable

The interface uses a template convention that facilitates the layering of multiple utilities to trivialize the implementation of otherwise intricate scripts, exemplified TODO

* Ease of Benchmarking 

Various *implementations* of the same object can be quickly and automatically benchmarked, saving money and time on expensive projects.

* Minimal Code

* Minimal Setup

This is a header library that only depends on the GNU Multiplle Precision Library. Inclusion into projects is done via `add_directory(extern/pinyon)` and `#include <pinyon.hh>`. 

# Documentation
The library is extensively documented and the points above are expanded therein. The organization of the abstraction/modularity layers is reflected by the directory structure, at the most basic leval toured by `src/`.

The interface is codified with the use of C++20 `Concepts` which serve as another for of documentation.

# Installation
The tests and benchmarking tools can be built via
```
sudo apt install libgmp3-dev
git clone --recurse-submodules https://github.com/baskuit/pinyon
cd pinyon
mkdir build
cd build
cmake ..
make
```
