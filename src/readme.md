# Search in the Abstract
* Don't go over motivation too much. J

This library was written in extreme generality to address several important constraints on development:

* What State? No sims, many formats
* What Model? 
* What Search algorithm? Not clear if MCTS or Stockfish like approach is best
* General performance: Tree nodes, mutexes, raw data types, etc



# Implementation

This section is intended to prepare the user for modifying and adding code in the pinyon library. P

## Why C++

* Popularity
* Libtorch support
* Templates, static vs dynamic polymorphism



## Type List Idiom

This idiom describes how variability of types is handled in the code

### Concepts & Intellisense

This library was developed using VSCode with C++ intellisense. This 

#### Enabling
Concepts can be disabled by uncommenting `#define ENABLE_CONCEPTS` in the file `types/concepts.hh`. All uses of concepts to constrain a template parameter are invoked using a macro.
```cpp
template  <CONCEPT(IsPerfectInfoStateTypes,  Types),  bool  HasPolicy  = false>
struct  MonteCarloModel  :  Types
```
If `ENABLE_CONCEPTS` is defined then `CONCEPT(IsPerfectInfoStateTypes, Types)` will expand to
```cpp

```
otherwise it will expand to a normal unconstrained template parameter.

This library has little for use concepts outside of the self-documentation provided by Intellisense. In fact, concepts generally make it harder to debug. Thus this macro was included to easily disable them across the entire project.

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



