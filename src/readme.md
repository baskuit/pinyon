
# Search in the Abstract
This library was developed as a platform for research and incremental progress in the development of computer-play in Pokemon. This means code should be reusable for multiple gens and simulators, with tools to compare evaluation functions and search algorithms. The library has been applied to `pmkn/engine` but I was not aware of any extant simulators besides Pokemon Showdown when I started development. With no compatible simulators to cater development towards, the library had to comply with any new simulators that could appear later.
 
This context lead me to abstract the machinery for search and reinforcement learning into distinct categories. These are orthogonal by design, so that different types within one category can be swapped with minimal restrictions.
These categories are:

* `TypeList`
	
	This encompasses the union of all primitive types and types which are essential for the implementation of the other categories. The short explanation is that these are the types you tweak to optimize performance. Support for `mpq_class` rationals as the `Real` or `Prob` type is the main exception to this rule, since arbitrary precision arithmetic is generally limited to more theoretical contexts.

* `State`

	Any simulator, game, or environment with a C/C++ interface can be wrapped as a state and subjected to Pinyon's utilities.

* `Model`

	This encompasses anything that provides a value and/or policy estimate for a game. It is generally expected that this information will be used in a tree search to produce a more refined result. The three most popular and generally successful approaches are listed below with their exemplars in Chess:
	* Heuristic (Stockfish < 12)
	* Large NN on GPU (AlphaZero, LeelaChess0)
	* Small NN on CPU (Stockfish 12+)

	Pinyon was designed to facilitate the development of all of these kinds of models. 

* `Search`
	Stockfish and Leela Chess exemplify the most promising methods for search: AlphaBeta solving and MCTS. The former is a depth first approach that uses game theoretic reasoning to prune as much of the game tree to produce its answer. Pinyon has an *improved* implementation of Simultaneous Move AlphaBeta for stochastic games, which means it is likely the SOTA in this field. The latter approach is more commonly known but fatally misunderstood in the simultaneous move context. Pinyon provides theoretically sound variants in a way that makes writing and benchmarking new algorithms easy.

* `Node`

	The performance of tree operations and consequently search is highly sensitive to cache use. The various tree structure implementations vary considerably in this aspect. 

# Language and Development Environment
C++ was the natural choice for this library for several reasons. 
* The bitter truth of machine learning and computer search is that performance is king, and C++ is as fast as any other language. 
* The 'interchangeability' of the type list, state, model etc. categories promised earlier is an example of *polymorphism*, and the template meta-programming system of C++ is a powerful way to approach this. This is because the template system is compile-time, so there is is no run-time performance cost to general code.
* It is among the most popular languages which means there is a wealth of references and support for new developers.
* C++ has excellent support for CUDA and large neural networks via Libtorch, the back-end for Pytorch. The latter is the most popular library for machine learning and shares an API with Libtorch.

## Building

This section is a walk-though of an installation using vscode on Linux.

### VSCode
VSCode is a free IDE that provides many useful features with minimal setup and it is highly recommended. A basic installation with the C/C++ and CMake extensions will make it easy to build, run and debug this library.

### CMake
CMake is the blessed build system for Pinyon, lrslib, and Libtorch. The  vanilla Pinyon CMakeLists.txt is setup so that configuring and enabling the Libtorch library only needs changes to a few lines.

```c
option(ENABLE_TORCH "Enable Libtorch"  OFF)
option(ENABLE_CONCEPTS "Enable Concepts"  ON)
```

The CMakeLists.txt includes a rudimentary script that scans the tests and benchmark directories for any source files. These executables can be built, testing and debugged via hotkeys with the CMake extension for VSCode.

### Compiler
Without excising any features or tests, this library requires a compiler that supports C++23. This is because of the use of `std::cartesian_product`, and use of concepts (detailed below) requires C++20, The core of the library is probably complicit with C++17, however.
* GCC
You will need at least `gcc-13`. As of Ubuntu 23.04, this can be installed by using `apt`. Older versions of Ubuntu and other distributions will probably require building the compiler from source.
* clang
As of this writing, there is a [bug](https://gcc.gnu.org/bugzilla//show_bug.cgi?id=109647) with clang-16 regarding libstdc++ version of the ranges library.
The library will compile with clang-17 (:warning: not true for all tests, there is a clang bug that appears with algorithm-generator.hh. TODO try clang-18)

### Concepts & Intellisense

The C++ extension for VSCode has autocomplete via Intellisense, and this has support for constrained template parameters. This means that in the following incomplete code:
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

The type list is the most important design pattern of Pinyon. It is used in the implementation of general purpose utilities where it manages the layers of abstraction. It is also used at the level of the executable, where it encapsulates a particular implementation of the search process.
In C++ terminology, it is a struct with no data members, only type and template aliases. 
The term "type list" specifically refers to the struct type definition, not any object/instance of the struct.

## Using a Type List

Suppose that we have a type list which defines enough types to run a simple search on a test game. This type list must define in its scope some types with the canonical aliases `State`, `Model`, `Search`, etc. These are essentially keywords in the Pinyon API. For the sake of example, lets say the search is a exp3 search on a "mold state" parameterized with 2 actions and a max length of 10. We will use a Monte-Carlo model in the search.

```cpp
int main () {
	using Types = //...
	size_t max_actions = 2;
	size_t depth = 10;
	Types::State state{max_actions, depth};
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

Now let's say we want to conduct a slightly different search. Now we want to use the `EmptyModel` instead of the Monte-Carlo model.  
This requires the use of a new type list. Obviously the `Model` type will be different in the new type list, but so will the `Search` type since its `run` method expects a different type of model. 
```cpp
// Code that uses different type list
using NewTypes = //...

```
If we were to run both of these examples, we would find that the latter type list's search runs faster. This is not surprising since the `EmptyModel` is basically a no-op. However it does hint at the many possibilities multiple type lists can offer.
One example is given by `eee.cc` TODO
```cpp
#include <pinyon.hh>

auto bandit_type_pack = TypePack<Exp3<MonteCarloModel<MoldState<>>>>{};
auto node_template_pack = NodeTemplatePack<DefaultNodes, LNodes, DebugNodes, FlatNodes>{};
auto search_type_tuple = search_type_generator<TreeBandit>(bandit_type_pack, node_template_pack);

int main () {
	// ...
}
```
This snippet uses a library utility to produce a `std::tuple` that has various type lists as elements.
This is the exception to the claim earlier that type lists are not intended to be initialized. However the rest of the script only uses the type list object to infer to type list.
The collection of type lists created by the `search_type_generator` in the example above differ only in their choice of node implementation.

## Building a Type List

The prior examples assumed that we had the type lists a priori. Now we will discuss how type lists are created

### The TypeList
Most type lists begin by specifying the struct definition of the "basic type list", which are the type lists that make up the TypeList category at the beginning of this document. 
These type lists don't contain definitions for the State or other features. They only specify the primitive types like `Real`, `Action`, `Mutex` that are used everywhere else.

### Templates
Now lets expand this type list to 

```cpp
template <typename Types>
struct MoldState : Types {
	class State {
		// implementation. define get_actions, apply_actions, etc.
	};
};
```


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

## 'Keyword' Types
The following declarations should be avoided by the user. They are reserved by the API.

* TypeList, Q, Real, Prob, Obs, ObsHash, Action, Mutex, PRNG, Seed, VectorReal, VectorAction, VectorInt, MatrixReal, MatrixInt, MatrixValue
* State
* Model, ModelOutput, ModelBatchInput, ModelBatchOuput
* Search, MatrixNode, ChanceNode, MatrixStats, ChanceStats

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



