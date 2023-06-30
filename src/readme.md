


# Generic Search

Surskit was designed to test search algorithms, which means it must accomodate all the well-studied approaches to game playing in the perfect information regime.
I chose to emulate proven methods in chess. For this, [LeelaChessZero](https://lczero.org/) and [Stockfish](https://stockfishchess.org/) were the inspiration. Both have achieved fantastic results in computer chess while taking drastically different approaches to search.

* `State`
Development on Surskit began before there were any fast Pokemon simulators available, so already there is no concrete game that we can assume as a base. Additionally, within Pokemon there are many different generations and formats of interest. Each is fundamentally a distinct game. 
Furthermore, the techniques and algorithms we want to investigate are not limited to any specific game.
* `Model`
We borrow this term from machine learning, where it typically refers to a trained neural network.
Search is simply looking ahead; we still need a way to estimate the game value of the future states our search algorithm visits. This is left to the 'Model'
There are three major categories in computer game playing. The oldest and simplest is the handcrafted heuristic. Conventional chess engines have used this approach until relatively recently, and it's safe to say that most bespoke game-playing programs use this too. The second is the large neural network, a la Alpha Zero. The appeal of neural networks is that, with a reasonable training regimen it should be difficult to end up with something catastrophically bad.  Finally we have a smaller neural network like the [NNUE](https://en.wikipedia.org/wiki/Efficiently_updatable_neural_network), which powers the recent versions of Stockfish. These networks tend to have less powerful inference than the former but they compensate with increased performance. Their inferences per second can be orders of magnitude larger.
It was not clear from the onset which approach is the most auspicious.
* `Algorithm`
In the perfect info case, there are two methods. Conventional chess engines use a solver, which visits each node in a sub-tree of the game at most once. There is also the MCTS approach, 
Ancillary types:

* `TypeList`
This may be thought of as an antecedent to the `State` family.  It encapsulates the various primitive and object data types that are used everywhere. The variability of the state types `Action`, `Observation` are self-evident. 

* `Node`
This family was only recently given more than one implementation. The performance of the search is highly sensitive to the tree structure. 

## Hierarchy

The families of types above can be ordered by dependency.

> TypeList < State < Model < Algorithm < Node

The methods of a family of types necessarily refer to the concrete types of the lower family
```cpp
State::apply_actions(Action row_action, Action col_action);

Model::get_inference(State& state, ModelOutput& output);

Algorithm::run(State& state, Model& model);
```

## Advantages of Modularity

### Testing

### Uncertainty

### Tuning

# Realizing

The above is a useful framework for thinking about search abstractly, but it must be implemented in a performant manner to be useful

## C++

For many reasons, C++ was a natural choice for this library.

* As a compiled language, it is orders of magnitude faster that interpreted languages like Python and NodeJS. It cannot be overstated how important this is for search to have a chance at producing strong play. Compilers for C++ are also highly sophisticated and can perform clever optimizations that can mitigate the cost of high abstraction.
* Stockfish and LC0 are both implemented in C++.
* C++ is one of the most popular compiled languages. This is important since this is a library. Other people should use it!
* Lots of resources for learning


In programming, the capability of the code to work with different types and implementations is called *Polymorphism*

The paradigm of type families at the start of this document must be codified in C++'s static typing. This is a non trivial design challenge and it took several refactors until I devised a convention that I felt was simple but expressive enough to achieve the modularity described above.

## A naive approach

C++ provides a simple scheme for polymorphism using class inheritance.

As a way to refer to a family of types in general, we define an abstract base class for each family. Each actual type in the family will derive from that abstract class, which establishes a shared interface (`get_actions` for states, `get_inference` for models, etc.)
```cpp
class State {
	virtual void get_actions () = 0;
};

class Battle : public State {
	void get_actions () override {
		// ...
	}
};

// Rollout method 
void rollout (State& state) {
	while (!state.is_terminal) {
		state.get_actions();
		Action row_action, col_action;
		// select random actions for both row and column players
		state.apply_actions(row_action, col_action);
	}
}

int main () {
	Battle battle{};
	rollout(battle);
	// battle is accepted as an arg because the `Battle` type is derived from the `State` type
}
```
This pseudo code demonstrates how the random rollout process, which is used in the Monte Carlo model, could be implemented so that it applies to any state. 

The `get_actions` method of the base class is marked `virtual`, which means it's intended to be called on an instance of a class *derived* from `State`, and that class will have its own implementation of the method.

This approach has a fatal flaw however. This kind of polymorphism is called *dynamic* because type checking is performed at run-time. Each time the program encounters a virtual method, it must lookup the implementation in a *v-table*. This operation takes time and seriously compromises performance due to the high number of virtual function calls in generic code.

## Templates to the Rescue

Templates in C++ allow a generic class or function to be defined with a type or multiple types as a parameter.
```cpp
template <typename State>
void rollout (State &state) {
	while (!state.is_terminal) {
		// state.apply_actions(); etc
	}
}
```
Thus instead of the program performing a vtable lookup at runtime, the different invocations or *specializations* of the template are determined during compilation.
```cpp
int main () {
	Battle battle{};
	rollout(battle); // bound to the function rollout<Battle> at compile-time.
``` 
This feature is extremely powerful and besides solving the demands of type-correctness, **it allows many classes to act as generic implementations of high level transformations**
E.g.
```cpp
	using Model = MonteCarloModel<Battle>;
	// type alias for brevity
	Model model{};
```
The Monte Carlo estimation method is encapsulated as a model and can be applied to *any* type with the state interface. If a class is missing the method `void apply_action(Action, Action)` or the property `is_terminal`, etc. then there will be a compilation error.
```cpp
	const int depth_to_solve = 3;
	TraversedState<Model> solved_battle {battle, model, depth_to_solve};
	// Expands the game tree rooted at the battle up to depth 3 
	// and solves for Nash Equilibrium strategies and payoff.
```
These generic transformations and classes have the same performance as if they were hard-coded for just one type.
	
# Implementation Details


The expressiveness of templates allows for many different solutions. When applied to an entire library of code, these solutions become conventions that should be followed as functionality as added. 

One of the simplest conventions and the one adopted here follows from considering the ordering we gave the families of types earlier.

> When 
# Idioms


```cpp
template <class Lower>
class UpperDerived : public UpperBase<Lower> {
    struct SomeStruct;
    struct Types :: UpperBase<Lower>::Types {
        using SomeStruct = UpperDerived::SomeStruct;
    };
};
```
If abstraction is not your preference, we can give a specific example

```cpp
```
In `AbstractUpper` we derive the Types struct
