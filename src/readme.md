
# Search in the Abstract
This library was developed as a platform for research and incremental progress in the development of a computer-play in Pokemon. This means code should be reusable for multiple gens and simulators, with tools to compare evaluation functions and search algorithms. The library has been applied to `pmkn/engine` but I was not aware of any extant simulators besides Pokemon Showdown when I started development.
This context lead me to abstract the setting and machinery for search and reinforcement learning into distinct categories. These are orthogonal by design, so that different types within one category can be swapped with minimal restrictions.
These categories are:
* `TypeList`
	
	This encompasses the union of all primitive types and types which are fundamental to the implementation of all other families. The short explanation is that these are the types you tweak to optimize performance. Support for `mpq_class` rationals as the `Real` or `Prob` type is the main exception to this rule, since arbitrary precision is generally limited to more theoretical contexts.
* `State`

	Any simulator, game, or environment with a C/C++ interface can be wrapped as a state and subjected to Pinyon's utilities.
* `Model`

	Anything that provides a value and/or policy estimate for a state. It is generally expected that this information will be used in a tree search to produce a more refined result. The three most popular and generally successful approaches are listed below with their exemplars in Chess:
	* Heuristic (Stockfish < 12)
	* Large NN on GPU (AlphaZero, LeelaChess0)
	* Small NN on CPU (Stockfish 12+)
* `Search`

	Stockfish and AlphaZero juxtapose the two most prominent families for lookeahead in perfect information games.
* `Node`

	The performance of tree operations and consequently search is highly sensitive to cache use. The various tree structure implementations vary considerably in this aspect. 

# Language and Development Environment
C++ was the natural choice for this project for several reasons. 
The bitter truth of machine learning and computer search is that performance is King, and C++ is as fast as any other language. 
The 'interchangeability' of the type list, state, model etc. categories promised earlier is known to computer science as *polymorphism*, and the template meta-programming feature of C++ is a powerful way to approach this. It even has the benefit of being *static* or compile-time, so there is is no performance cost to general code.
It is among the most popular languages
C++ has excellent support for CUDA and large neural networks via Libtorch, the back-end for Pytorch. The latter is the most popular library for machine learning and it shares an API with Libtorch.

## Building

This section is a walk-though of an installation using vscode on Linux.

### VSCode

VSCode is a free IDE that provides many useful features with minimal setup and it is highly recommended. A basic installation with the C/C++ and CMake extensions will make it easy to build, run and debug this library.
### CMake
CMake is the blessed build system for Pinyon, lrslib, and Libtorch. The  vanilla Pinyon CMakeLists.txt is setup so that configuring and enabling the Libtorch library only needs changes to a few lines.

```c
option(ENABLE_TORCH "Enable Libtorch"  OFF)
```

The CMakeLists includes a rudimentary script that scans the tests and benchmark directories for any source files. These executables can be built, testing and debugged via hotkeys with the CMake extension for VSCode.

### Compiler
Without excising any features or tests, this library requires a compiler that supports C++23. This is because of the use of `std::cartesian_product`, and use of concepts (detailed below) requires C++20, The core of the library is probably complicit with C++17, however.
* GCC
You will need at least `gcc-13`. As of Ubuntu 23.04, this can be installed by using `apt`. Older versions of Ubuntu and other distributions will probably require building the compiler from source.
* clang
As of this writing, there is a [bug](https://gcc.gnu.org/bugzilla//show_bug.cgi?id=109647) with clang-16 regarding libstdc++ version of the ranges library.
The library will compile with clang-17 (:warning: not true for all tests, there is a clang bug. TODO try clang-18)

### Concepts & Intellisense

The C/C++ extension for VSCode has autocomplete via Intellisense, and this has support for constrained template parameters. This means that in the following incomplete code:
```cpp
template <IsPerfectInfoStateTypes State>
void method_for_state_types(
	Types::State &state) {
	state.
``` 
the IDE would suggest the methods `is_terminal()`, `apply_actions()` etc.
This provides a form of documentation that should help users become familiar with the Pinyon interface.


#### Disabling Concepts
Concepts can be disabled by uncommenting `#define ENABLE_CONCEPTS` in the file `libpinyon/enable-concepts.hh`. All uses of concepts for Intellisense are invoked using a macro, e.g.
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

#### As Documentation

The atomic constraints that define the various concepts are a serviceable tour of the interface. The rest of the readme files will reiterate and comment on the constraints.


# The Types Idiom

The type list is the most important pattern of Pinyon. It is used in the implementation of general purpose utilities where it lends a consistent way. It is also used at the level of the executable, where encapsulates a particular.
In C++ terminology, it is a struct with no data members, only type and template aliases. 
The term "type list" specifically refers to the struct type definition, not any object/instance of the struct.

## Using a Type List

Suppose that we have a type list which defines enough types to run a simple search on a test game. Then this type list must define in its scope some types with the canonical aliases `State`, `Model`, `Search`, etc. These are essentially keywords pinyon API. For the sake of example, lets say the search is a exp3 search on a "mold state" parameterized with 2 for the players and a max depth of 10. We will use a monte carlo model in the search.

```cpp
int main () {
	using Types = //...
	size_t depth = 10;
	Types::State state{depth};
	Types::Model model{0};
	Types::Q gamma{1, 10};
	Types::Search search{gamma};
	Types::PRNG device{0};
	size_t time_ms = 500;
	Types::MatrixNode node{};
	
	size_t i = search.run(time_ms, device, state, model, node);
	std::cout << i << " iterations performered in " << time_ms << " milliseconds." << std::endl;
}
```

### Multiple Type Lists

Now what about using slightly different type. Say we want to do this same test but we want to use 

## Building a Type List

The prior examples assumed that we had the type lists a priori. Now we will discuss how type lists are created

### The TypeList

Most type lists begin by specifying the struct definition of the "basic type list", which are the type lists that make up the TypeList category at the beginning of this document. 
These type lists don't contain definitions for the State or other features. They only specifiy the primitive types like `Real`, `Action`, `Mutex` that are used everywhere else.

### Templates

### Relation to Concepts


```cpp
template <>
```

# Other Idioms

## Row and Column

The assumption of perfect information  means that neither players perspective is special. Any lookahead or search procedure either player can do is equivalent to one the other player can perform.
Still we need to distinguish between the two players in the library code so we use a naming convention that ...
The code thus has the `row` and `col` prefixes everywhere e.g. `row_gains` vs `col_gains`, `row_payoff` vs `col_payoff`, etc.

When (If) support for imperfect information is added the row player will be the 'owner' of the search. Search will have access to the row player's private information but not necessarily the column player's.

## Keyword Types
* TypeList
* State
* Model, ModelBatchInput, ModelBatchOuput
* Search, MatrixNode, ChanceNode

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
