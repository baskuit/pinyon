
# Pinyon
Pinyon is a high performance library for research and development of search and solving algorithms for perfect-info, simultaneous move, stochastic games. The code emphasizes:

* Compile time

* Full Modularity

* Ease of benchmarking 

* Minimal code/clarity of ideas

* Minimal dependencies, ease of use

# Overview

### Library Utilities

* Powerful class of test games
* GMP support for precise numbers
* Fast NE solving

Most importantly, the sum total of all these optinos are contained as a type list.
This is the most important part of Pinyon. 


# Installation
This project makes heavy use of the `Concepts` feature of C++20. Furthermore, the tests occasionally use `std::cartiesian_product` from C++23. Support for the latter is spotty, but GCC-13 will work
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
