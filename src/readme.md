
# Search in the Abstract
This library was developed as a platform for research and incremental progress in the development of a computer-play in Pokemon. This means code that is reusable for all gens and simulators, and tools to evaluate disparate search algorithms. The library has been applied to engine but I was not aware of any extant simulators besides Pokemon Showdown when I started development.
This context lead me to abstract the setting and machinery for search and reinforcement learning into distinct categories. These are orthogonal by design, so that a method belonging to one category can be applied to pieces with minimal restrictions.
These groups are listed below:
* TypeList
	This encompasses the union of all primitive types and types which are fundamental to the implementation of all other families. The short explanation is that these are the types you tweak to optimize performance. Support for `mpq_class` rationals as the `Real` or `Prob` type is the main exception to this rule, since arbitrary precision is generally limited to more theoretical contexts.
* State
Any simulator, game, or environment with a C/C++ interface can be wrapped as a state and subjected to Surskit's utilities.
* Model
Anything that provides a value and/or policy estimate for a state. It is generally expected that this information will be used in a tree search to produce a more refined result. The three most popular and generally successful approaches are listed below with their exemplars in Chess:
	* Heuristic (Stockfish < 12)
	* Large NN on GPU (AlphaZero, LeelaChess0)
	* Small NN on CPU (Stockfish 12+)
* Search
Stockfish and AlphaZero juxtapose the two most prominent families for lookeahead in perfect information games.
* Node
The performance of tree operations and consequently search is highly sensitive to cache use. The various tree structure implementations vary considerably in this aspect. 

# Language and Development Environment
C++ was the natural choice for this project on many fronts. 
The bitter truth of machine learning and computer search is that performance is King, and C++ is as fast as any other language. 
The 'interchangeability' of the type list, state, model etc. categories promised earlier is known to computer science as *polymorphism*, and the template meta-programming feature of C++ is a powerful way to approach this. It even has the benefit of being *static* or compile-time, so there is is no performance cost to general code.
It is among the most popular languages
C++ has excellent support for CUDA and large neural networks via Libtorch, the back-end for Pytorch. The latter is the most popular library for machine learning and it shares its interface with Libtorch.

## Building

It's not anticipated that users are experienced C++ programmers. This section provides instructions to mirror the development envir

### VSCode

VSCode is a free IDE that provides many useful features with minimal setup. It is highly recommended for users that are new to C++ development. A basic installation with the C/C++ and CMake extensions will make it easy to build, run and debug this library.
### CMake
CMake is the blessed build system for Surskit, lrslib, and Libtorch. The  vanilla Surskit CMakeLists.txt is setup so that configuring and enabling the Libtorch library only needs changes to a few lines.

```cmake
option(ENABLE_TORCH "Enable Libtorch"  OFF)
```

It also comes with a rudimentary script that scans the tests and benchmark directories for any source files. These executables can be built, testing and debugged with a a simple button press using the CMake extension.

### Compiler
Without excising any features or tests, this library requires a compiler that supports C++23. This is because of the use of `std::cartesian_product`, and use of concepts (detailed below) requires C++20, The core of the library is probably complicit with C++17, however.
#### GCC
You will need at least `gcc-13`. As of Ubuntu 23.04, this can be installed by using `apt`. Older versions of ubuntu and other distros will probably require building the compiler from source.
#### clang
As of this writing, there is a [bug](https://gcc.gnu.org/bugzilla//show_bug.cgi?id=109647) with clang-16 regarding libstdc++ version of the ranges library.
The library will compile with clang-18 and probably clang-17 (untested).

* VSCode
* Compiler
* CMake
* Additional libraries

This project its dependencies LRSNashLib are built using 


### Concepts & Intellisense

The C/C++ extension for VSCode has autocomplete via Intellisense, and this has support for constrained template parameters. This means that in the following incomplete code:
```cpp
template <IsPerfectInfoStateTypes State>
void method_for_state_types(
	Types::State &state) {
	state.
``` 
the IDE would suggest the methods `is_terminal()`, `apply_actions()` etc.
This provides a form of documentation that should help users become familiar with the Surskit interface.


#### Disabling Concepts
Concepts can be disabled by uncommenting `#define ENABLE_CONCEPTS` in the file `types/concepts.hh`. All uses of concepts for Intellisense are invoked using a macro, e.g.
```cpp
template <CONCEPT(IsPerfectInfoStateTypes, Types),  bool HasPolicy = false>
struct MonteCarloModel : Types { // ...
```
The template parameter definition will expand to
```cpp
template <IsPerfectInfoStateTypes Types,  bool  HasPolicy  = false>
```
if `ENABLE_CONCEPTS` is defined. Otherwise it will expand to a normal unconstrained template parameter:
```cpp
template <typename Types,  bool  HasPolicy  = false>
```
This library has little for use concepts outside of Intellisense. In fact, concepts generally produce worse compiler messages. Thus this macro was included to easily disable them across an entire project.


# The Types Idiom

The most important implementation detail of Surskit, because it is used everywhere and thus provides a uniform API, is the *type list*.

A *type list* is a struct which only contains alias and struct/class declarations. This is usually a minimal collection, only containing enough types to define a new class and perhaps some ancillary types as well.
```cpp
using Types = MoldState<2>;
// The rhs is a type list which we create an 'Types' alias for.	
MoldState<2>::State state{10};
// In this case MoldState only contains a struct definition `State`
// which is the name sake for the type list
Types::VectorAction row_actions, col_actions;
state.get_actions(row_actions, col_actions);
// and also the basic 'surskit types' which all type lists are assumed to have
```


 Usually a given type list is actually a specialization of a template that acts like a type list "expander": 
```cpp
template <IsValueModelTypes Types>
struct NewTypes : Types {
	using StateModelPair = std::pair<typename Types::State, typename Types::Model>
	// ...
};
```
This essentially acts like a function that takes a type list (which satisfies the `IsValueModelTypes` constraint) and defines a new one. In most cases (like the above) this new type lists will 'contain' the old one via class inheritance. It will then define new types, in the above with have the new `StateModelPair` type to work with.


# Tour of Features

The directory structure of the `/src` directory mirrors the classification of search functions into 

### `/types`
* ``

### `/state`
* `RandomTree`
* `TraversedState`
* `MoldState`
* `Arena`

### `/model`
* `MonteCarloModel`
* `EmptyModel`
* `LibtorchModel`
* `SearchModel`
* `SolvedModel`

### `/algorithm`
* TreeBandit
* Solving

### `/tree`



