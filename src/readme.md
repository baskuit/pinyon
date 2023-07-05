


# Generic Search

Surskit was designed to test search algorithms, which means it must accomodate all the well-studied approaches to game playing in the perfect information regime.

* `State`
Development on Surskit began before there were any fast Pokemon simulators available, so already there is no concrete game that we can assume as a base. Under the umbrella of "competitive", there are many different generations and formats that are fundamentally distinct games. 
Furthermore, the techniques and algorithms we want to investigate are not limited to any specific game.
* `Model`
We borrow this term from machine learning, where it typically refers to a trained neural network.
Search is simply looking ahead; we still need a way to estimate the game value of the future states our search algorithm visits. This is left to the 'Model'.
There are three major inference schemes in computer game playing. The oldest and simplest is the handcrafted heuristic. Conventional chess engines have used this approach until relatively recently, and it's safe to say that most bespoke game-playing rely on this too. This approach is the simplest for three and has the potential to be the fastest, which is critical for playing strength.
The second is the large neural network, a la Alpha Zero. One advantage of neural networks is that they are capable of much more sophisticated inference than a manual device. This approach was dominant in computer chess until the advent of the small [NNUE](https://en.wikipedia.org/wiki/Efficiently_updatable_neural_network). These networks tend to have less powerful inference than the former but they compensate with increased performance. Their inferences per second can be orders of magnitude larger.

* `Algorithm`

I chose to emulate proven projects in chess. For this, [LeelaChessZero](https://lczero.org/) and [Stockfish](https://stockfishchess.org/) were the inspiration for the 'tree-bandit' and 'solver' folders respectively. Both have achieved fantastic results in computer chess while taking drastically different approaches to search.

The former is a generalization of the 'Monte Carlo' tree search which was proven with AlphaZero and afterward successfully applied to many domains.

The latter models the iterative deepening appraches of stockfish and most chess engines.

* `TypeList`
This may be thought of as an antecedent to the `State` family.  It encapsulates the various primitive and object data types that are used everywhere. Certain types like `Action` and `Observation` depend on the game in question or its implmentation. Others are variable for perforamance reasons or just to make the library more flexible and extensible. For example, the default prng device is a mersenne twister, which could easily be substituted from a faster scheme. This is as simple as including the implementation into the project and making a one line adjustment to the using declaration in the `Types` object.

* `Node`
This family was only recently given more than one implementation. The performance of the search is highly sensitive to the tree structure. In all search algorithms, a transition of a state is always paired with an equivalent operation of matrix nodes where a child node is produced from the parent.

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

## Language

C++ was a natural choice for this library.

* As a compiled language, it is orders of magnitude faster that interpreted languages like Python and NodeJS. It cannot be overstated how important this is for search to have a chance at producing strong play. Compilers for C++ are also highly sophisticated and can perform clever optimizations that can mitigate the cost of high abstraction.
* Stockfish and LeelaChessZero are both implemented in C++.
* C++ is one of the most popular compiled languages. This is important since this is a library. Other people should use it!


In programming, the capability of the code to work with different types and implementations is called *Polymorphism*

The paradigm of type families at the start of this document must be codified in C++'s static typing. This is a non trivial design challenge and it took several refactors until I devised a convention that I felt was simple but expressive enough for useful modularity.

## A naive approach

Polymorphism can be achieved in C++ using classes and inheritence:

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
This pseudo code demonstrates how the random rollout process, which is used in the Monte Carlo model, could be implemented so that it works for any state. 

The `get_actions` method of the base class is marked `virtual`, which means it's intended to be called on an instance of a class *derived* from `State`, and that class will provide its own implementation of the method.

This approach has a fatal flaw however. It is called *dynamic* polymorphism because type checking is performed at run-time. Each time the program encounters a virtual method, it must lookup the implementation in a *v-table*. This operation takes time and seriously compromises performance due to the high number of virtual function calls in generic search code.

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


The expressiveness of templates allows for many different solutions. When applied to an entire library of code, these solutions become conventions that should be followed as functionality is added. 

One of the simplest conventions and the one adopted here follows from considering the ordering we gave the families of types earlier.

> When defining a type (such as `MonteCarloModel` for example), we  the prerequisite type in the family just below as the first template parameter: `MonteCarloModel<Battle>`. This convention can be seen on clear display in the following boilerplate alias declarations
```cpp
using State = MoldState<9>;
using Model = MonteCarloModel<State>;
using Search = TreeBandit<Exp3<Model>>;

const size_t depth = 20;
State state{depth};
Model model{};
const double gamma = .1;
Search session{gamma};
```

If we unroll these alias declaration, we can see that the type of the final search object is
`TreeBandit<Exp3<MonteCarloModel<MoldState<9>>>>`!

How exactly are the identities of all the lower types managed in the code of the higher families?

## Types Struct

(Nearly) every class in Surskit has a special `Types` struct defined in its class scope. The `TypeList` classes are the only ones without this struct because they essentially are a types struct.

```cpp
template <class Model>
class Exp3 : public AbstractAlgorithm<Model> {
	// ...
};
```

Consider the code for the Exp3 algorithm above. We've explicity declared the type of the 'model' class to simply be the `Model` template parameter type. In scope of the class, we can simply use this type directly

```cpp
	void get_inference (Model &model) {
		// contrived example
	}
```

But then how would we refer to the lower types? We could also pass the lower types as a template parameter, but that would quickly produce very ugly code.

```cpp
using State = MoldState<9>;
using Model = MonteCarloModel<State>;
using BanditAlgorithm = Exp3<Model, State>
using Search = TreeBandit<BanditAlgorithm, Model, State>;
```
Even worse than before!

The solution is to 
```cpp
	typename Types::Real gamma = .01;
	typename Types::VectorAction row_actions, col_actions;
// same real and vector type that was declared in the TypesStruct
	void run (
		const size_t iterations,
		const typename Types::State &state,
		const typename Types::Model &model,
		MatrixNode *matrix_node
	) {
		// ...
	}
```

The idiom `typename Types::Name` essentially means that within the `Types` struct, there is some declaration

`using Name = // Some concrete type specifiation`

for each possible lower type. These declarations are only made once, and they are then inherited as we go up the hierarchy of type families.

## Ancestry of the Types Struct

The most primitive declaration are made in the very first family, the TypesList. See the default implementation below.

```cpp
// template param declarations here
struct Types
{
    using Rational = _Rational;
    using Real = RealType<_Real>;

    using Action = ActionType<_Action>;
    using Observation = ObservationType<_Observation>;
    using ObservationHash = ObservationHashType<_Observation>;
    using Probability = ProbabilityType<_Probability>;

    using Seed = _Seed;
    using PRNG = _PRNG;

    using Value = ValueStruct<Real, true>;

    using VectorReal = _VectorReal<Real>;
    using VectorAction = _VectorAction<Action>;
    using VectorInt = _VectorInt<int>;

    using MatrixReal = _MatrixReal<Real>;
    using MatrixInt = _MatrixInt<int>;
    using MatrixValue = Matrix<Value>;

    using Strategy = VectorReal;
};
```

Thus `typename Types::Value` in will be deduced by the compiler to mean `_ValueStruct<Real, true>`.

Lets see how these declarations become accessible to the implementation of `RandomTree`.

```cpp
template <typename _Types = RandomTreeTypes>
class RandomTree : public ChanceState<_Types>
{
public:
    struct Types : ChanceState<_Types>::Types
    {
    };

    typename Types::PRNG device;
    typename Types::Seed seed{};
	//...
};
```
In the scope of `RandomTree` the name Types referes to the struct that was just declared (as always) at the top of the class block. This struct does not have any *new* declarations in its body, but it did inherit the declarations from `ChanceState<_Types>::Types`, which is a struct named `Types` that was declared in the scope of `ChanceState`. We can follow this inheritance all the way back to the original.

```cpp
template <class _Types>
class ChanceState : public PerfectInfoState<_Types>
{
public:
    struct Types : PerfectInfoState<_Types>::Types
    {
    };
```

```cpp
template <class _Types>
class PerfectInfoState : public AbstractState<_Types>
{
public:
    struct Types : AbstractState<_Types>::Types
    {
    };
```

```cpp
template <class _Types>
class AbstractState
{
public:
    struct Types : _Types
    {
        using TypeList = _Types;
    };
```

Thus the chain of inhertence for `RandomTree<RandomTreeTypes>::Types` is

`RandomTree<RandomTreeTypes>::Types`

`ChanceState<RandomTreeTypes>::Types`

`PerfectInfoState<RandomTreeTypes>::Types`

`AbstractState<RandomTreeTypes>::Types`

`RandomTreeTypes`

which does in fact contain the declarations


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
