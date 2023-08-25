# Search in the Abstract
This library was developed as a way to attack the problem of Pokemon A.I. as efficiently as possible. This means code that is reusable for all gens and simulators, and tools to evaluate disparate search algorithms. The library has been applied to engine but I was not aware of any extant simulators besides Pokemon Showdown when I started development.
This context lead me to abstract the machinery for search and reinforcement learning into distinct families.
* TypeList
	This encompasses the union of all primitive types and types which are fundamental to the implementation of all other families. The short explanation is that these are the types you tweak to optimize performance. Support for `mpq_class` rationals as the `Real` or `Prob` type is the sole exception  to this rule, since arbitrary precision types are generally used in theory contexts.
* State
Any simulator, game, or environment with a C/C++ interface can be wrapped as a state and subjected to Surskit's utilities.
* Model
This is essentially anything that provides a value and/or policy estimate for a State. There are 3 major approaches for this task, and these are best introduced by exemplars in the Chess domain:
	* Heuristic (Stockfish < 12)
	* Large NN on GPU (AlphaZero, LeelaChess0)
	* Small NN on CPU (Stockfish 12+)
	
	The algorithms included with Surksit should work with any of these 
* Search
I'm convinced there are two indicator candidates for search: fast adversarial bandits via exp3 and matrix-aware approaches using 

# Implementation

This section is intended to prepare the user for modifying and adding code in the pinyon library.

## Why C++

* Popularity
* Libtorch support
* Templates, static vs dynamic polymorphism



## Type List Idiom

This idiom describes how variability of types is handled in the code

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

### Concepts & Intellisense

This library was developed using VSCode with the official C++ extension. This extension has autocomplete via Intellisense, and this has support for constrained template parameters. This means that the following incomplete code
```cpp
template <IsPerfectInfoStateTypes State>
void method_for_state_types(
	Types::State &state) {
	state.
``` 
would suggest the methods `is_terminal()`, `apply_actions()` etc.
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
if `ENABLE_CONCEPTS` is defined. Otherwise it will expand to a normal unconstrained template parameter.
```cpp
template <typename Types,  bool  HasPolicy  = false>
```
This library has little for use concepts outside of Intellisense. In fact, concepts generally produce worse compiler messages. Thus this macro was included to easily disable them across an entire project.

# Tour of Features

### /Types
* ``

### /State
* `RandomTree`
* `TraversedState`
* `MoldState`
* `Arena`

### /Model
* `MonteCarloModel`
* `EmptyModel`
* `LibtorchModel`
* `SearchModel`
* `SolvedModel`

### /Algorithm
* TreeBandit
* Solving

### /Tree



